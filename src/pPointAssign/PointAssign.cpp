/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.cpp                                 */
/*    DATE: March 4th, 2025                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "PointAssign.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

PointAssign::PointAssign()
{
  m_ship_names ={};
  m_visit_points = {};
  m_assignment_method = "default"; //Odds and evens or East/west

  m_ship_reg_time = 0;

  m_start_flag = "firstpoint";
  m_end_flag = "lastpoint";

  m_first_reading = false;
  m_last_reading = false;
}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
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

    /*if(key == "NODE_REPORT") {
      //Collects unique ship names for distribution
      if(find(m_ship_names.begin(), m_ship_names.end(), msg.GetCommunity()) != m_ship_names.end()){
        m_ship_names.push_back(msg.GetCommunity());
        m_ship_reg_time = msg.GetTime();
     }
    }*/ //^^Draft Code for autodetecting ships

    if(key == "VISIT_POINT") { //Collects visit points, stops collecting once end reached
      if (msg.GetString() == m_start_flag) { //!!!!!!!CORRECT to possibly accept a new list later
        m_first_reading = true;
        m_visit_points.push_back(msg.GetString());
      } else if (msg.GetString()!= m_end_flag){
        m_visit_points.push_back(msg.GetString());
      } else if (msg.GetString() == m_end_flag){
        m_last_reading = true;
      } else {
        reportRunWarning("Unhandled Mail: " + key);
      }
    } // Add missing closing brace

    else if(key == "DB_TIME") {
      m_current_time = msg.GetDouble();
    }
    else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool PointAssign::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
  AppCastingMOOSApp::Iterate();
  //if(m_current_time - m_ship_reg_time > 10){ //To make sure enough time for all ships to register

  if(m_last_reading){ //Don't want to interate until list is complete
    retractRunWarning("Incomplete set of points provided");

    if(m_assignment_method == "default"){ //Default is alternate points, alt is East/West
      for(int i = 0; i < m_ship_names.size(); i++){ //Loop for Each Ship
        Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[0]); //Publishes "firstpoint"
  
        for(int j = 1; j < m_visit_points.size()-1; j++){ //Loop for list of points
          if(j % m_ship_names.size() == 0){
            Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[i]);
          }
        }

        Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[m_visit_points.size()-1]); //Publishes "lastpoint"
      }
    } else if(m_assignment_method == "false"){ //East West
      int x_coord;
      int y_coord;
  
      for(int i = 0; i < m_ship_names.size(); i++){ //Loop for Each Ship
        Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[i]);
  
  
        
      }
    } else {
      reportRunWarning("Incomplete set of points provided ");
    }

  }








  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp()
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
    if(param == "ship_list") {
      m_ship_names = parseString(value, ',');
      handled = true;
    }
    else if(param == "assignment_method") {
      m_assignment_method = value;
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

void PointAssign::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("VISIT_POINT",0);
  Register("NODE_REPORT",0);
  Register("DB_TIME",0);

}


//------------------------------------------------------------
// Procedure: buildReport()

bool PointAssign::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "I did not Set Up Yet                        " << endl;
  m_msgs << "============================================" << endl;

/*
  ACTable actab(4);
  actab << "Alpha | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << "one" << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();
*/
  return(true);
}

void PointAssign::postViewPoint(double x, double y, string label, string color){
  XYPoint point(x,y);
  point.set_label(label);
  point.set_color("vertex",color);
  point.set_param("vertex_size","4");
  Notify("VIEW_POINT",point.get_spec());
}



