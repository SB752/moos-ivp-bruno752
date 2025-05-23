/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.cpp                                        */
/*    DATE: February 20th, 2025                             */
/*    UPDATED: February 25th, 2025                           */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Odometry.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

Odometry::Odometry()
{
  m_first_reading = true;
  m_Xtarget = "NAV_X";
  m_Ytarget = "NAV_Y";
  m_current_time = 0;
  m_lastupdate_time = 0;
  m_current_x = 0;
  m_current_y = 0;
  m_previous_x = 0;
  m_previous_y = 0;
  m_total_distance = 0;
  m_errorcheck_time = 10;
  m_odometer_units = "default_unit";
  m_depth_thresh = 0;
  m_current_depth = 0;
  m_dist_at_depth = 0;
}

//---------------------------------------------------------
// Destructor

Odometry::~Odometry()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool Odometry::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

/*
#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
*/

    if(key == m_Xtarget) 
      m_current_x = msg.GetDouble();
    else if(key == m_Ytarget) { 
      m_current_y = msg.GetDouble();
     } 
    else if(key == "DB_TIME") {
      m_current_time = msg.GetDouble();
    }
    else if (key == "ODOMETRY_UNITS") {
      m_odometer_units = msg.GetString();
    }
    else if (key == "NAV_DEPTH") {
      m_current_depth = msg.GetDouble();
    }
    else if (key == "DEPTH_THRESH"){
      m_depth_thresh = msg.GetDouble();
    }
    else if (key == "OBJECTIVE_DIST"){
      m_objective_dist = msg.GetDouble();
    }

    else if (key == "APPCAST_REQ")  // handled by AppCastingMOOSApp
    {
      /* code */
    }
    

    else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool Odometry::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Odometry::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!

  if(!m_first_reading){
    if(m_current_x != m_previous_x && m_current_y != m_previous_y){
      m_lastupdate_time = m_current_time;
    }
      double distance = sqrt(pow(m_current_x - m_previous_x, 2) + pow(m_current_y - m_previous_y, 2));
      m_total_distance += distance;
      if(m_current_depth > m_depth_thresh){
        m_dist_at_depth += distance;
      }
      m_previous_x = m_current_x;
      m_previous_y = m_current_y;

      Notify("ODOMETRY_DIST", m_total_distance);
      //Notify("ODOMETRY_UNITS", m_odometer_units);
      Notify("ODOMETRY_DIST_AT_DEPTH", m_dist_at_depth);
      Notify("OBJECTIVE_DIST", m_objective_dist);

    }else if(m_first_reading) {
      m_previous_x = m_current_x;
      m_previous_y = m_current_y;
      m_first_reading = false;
    }

//Runtime Warning for no new data
  if(m_current_time - m_lastupdate_time > m_errorcheck_time){
    reportRunWarning("No new data in some time");
  } else{
    retractRunWarning("No new data in some time");
  }


  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Odometry::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());


  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "updatetime") { //Check for configuration parameter what time to wait before runtime warning
      m_errorcheck_time = atof(value.c_str());
      handled = true;
    }

    else if(param == "odometry_units") {
      m_odometer_units = value;
      handled = true;
    }

    else if(param == "depth_thresh") {
      m_depth_thresh = atof(value.c_str());
      handled = true;
    }
    else if(param == "objective_dist"){
      m_objective_dist = atof(value.c_str());
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables()

void Odometry::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register(m_Xtarget, 0);
  Register(m_Ytarget, 0);
  Register("DB_TIME", 0);
  Register("ODOMETRY_UNITS", 0);
  Register("NAV_DEPTH", 0);
  Register("DEPTH_THRESH", 0);
  Register("OBJECTIVE_DIST",0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool Odometry::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "Distance Travelled: "<< m_total_distance << " " << m_odometer_units << endl;
  m_msgs << "Distance Travelled at Depth: " << m_dist_at_depth << " " << m_odometer_units << endl;
  m_msgs << "============================================" << endl;

  return(true);
}




