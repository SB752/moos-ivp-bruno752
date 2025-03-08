/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.cpp                                        */
/*    DATE: March 7th, 2025                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "GenPath.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

GenPath::GenPath()
{
  m_mission_points;
  m_start_flag = "firstpoint";
  m_end_flag = "lastpoint";
  m_first_reading = false;
  m_last_reading = false;

  m_waypoints_output = "points = ";

  m_first_x = false;
  m_first_y = false;

  m_waypoints_published = false;

  m_score_count = 0;
  m_trouble_tracker =1;
}

//---------------------------------------------------------
// Destructor

GenPath::~GenPath()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool GenPath::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

     if(key == "VISIT_POINT"){ 
       if (msg.GetString() == m_start_flag){
         m_first_reading = true;
       } else if (msg.GetString() == m_end_flag){
         m_last_reading = true;
       } else {
         PointReader point;
         point.intake(msg.GetString());
         m_mission_points.push_back(point);
       }

     } else if(key == "NAV_X" && !m_first_x){
       m_x_pos = msg.GetDouble();
       //m_first_x = true;

     } else if(key == "NAV_Y" && !m_first_y){
       m_y_pos = msg.GetDouble();
      //m_first_y = true;

     } else if(key=="SCORE"){
      m_score_count += msg.GetDouble();
     }


     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool GenPath::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenPath::Iterate()
{
  AppCastingMOOSApp::Iterate();
  if(m_last_reading && !m_waypoints_published){  //Doesn't run until all points are collected
    double Prev_x = m_x_pos;
    double Prev_y = m_y_pos;
    vector<PointReader> copy = m_mission_points;

    Notify("PRE-LOOP",copy.size());

    //Sorts points by closest distance to previous point, starting with first point
    while(copy.size()>0){
      double min_dist = 1000000;
      int min_index = 0;
      Notify("ENTERED LOOP","Success");
      for(int i=0; i<copy.size(); i++){ //finds closest point
        double dist = sqrt(pow(m_mission_points[i].get_x()-Prev_x,2)+pow(m_mission_points[i].get_y()-Prev_y,2));
        if(dist < min_dist){
          min_dist = dist;
          min_index = i;
        }
      }
      copy.erase(copy.begin()+min_index); //removes closest point from list for remaining iterations


      Notify("MIS POINTS PROCESSED", m_trouble_tracker);
      m_trouble_tracker++;

      m_waypoints_output += to_string(m_mission_points[min_index].get_x()) + "," + to_string(m_mission_points[min_index].get_y()) + ":";
      Prev_x = m_mission_points[min_index].get_x(); //Resets previous point to closest point for next loop iteration
      Prev_y = m_mission_points[min_index].get_y();
      

    }
    //Sends waypoint list once
    Notify("UPDATES",m_waypoints_output);
    m_waypoints_published = true;
  }


  Notify("SCOREBOARD",m_score_count);
  AppCastingMOOSApp::PostReport();
  return(true);
}


//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenPath::OnStartUp()
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
    if(param == "foo") {
      handled = true;
    }
    else if(param == "bar") {
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

void GenPath::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("VISIT_POINT",0);
  Register("NAV_X",0);
  Register("NAV_Y",0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "# of Waypoints:                             " << endl;
  m_msgs << m_mission_points.size() << endl;
  m_msgs << "SCORE: "<< endl;
  m_msgs << m_score_count << endl;
  m_msgs << "============================================" << endl;

  return(true);
}




