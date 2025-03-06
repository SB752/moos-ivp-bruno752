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
  m_ship_points;

  m_assignment_method = "default"; //Odds and evens or East/west

  m_ship_reg_time = 0;

  m_start_flag = "firstpoint";
  m_end_flag = "lastpoint";

  m_first_reading = false;
  m_last_reading = false;

  m_point_pub_complete = false;

  trouble_counter = 0;
  trouble_iterate = 0;
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
  Notify("RECIEVED","Hello!!!!!!!!!!");
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();
    Notify("RECIEVED",key);

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
      m_visit_points.push_back(msg.GetString());
      string whatismessage = msg.GetString();
      Notify("LOOP", key);
      Notify("BAD_MESSAGE", whatismessage);
      trouble_counter++;
      Notify("TROUBLE_COUNTER", trouble_counter);
      string TBout = "vector_end"+std::to_string(trouble_counter);
      Notify(TBout, m_visit_points[m_visit_points.size()-1]);
      if (msg.GetString() == m_start_flag) {
        m_first_reading = true;
      } else if (msg.GetString() == m_end_flag){
        m_last_reading = true;
      }
    }

/*
    else if(key == "DB_TIME") {
      m_current_time = msg.GetDouble();
    }
    */
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
  Notify("ITERATE","Made it here");
  #if 0
  //if(m_current_time - m_ship_reg_time > 10){ //To make sure enough time for all ships to register

  if(m_last_reading && (!m_point_pub_complete)){ //Don't want to interate until list is complete
    retractRunWarning("Incomplete set of points provided");

    if(m_assignment_method == "default"){ //Default is alternate points, alt is East/West
      for(int i = 0; i < m_ship_names.size(); i++){ //Loop for Each Ship
        //Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[0]); //Publishes "firstpoint"
        m_ship_points[i][0] = m_visit_points[0]; //Stores first point for each ship
  
        for(int j = 1; j < m_visit_points.size()-1; j++){ //Loop for list of points
          if(j % m_ship_names.size() == (i-1)){
            //Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[j]);
            m_ship_points[i][j] = m_visit_points[j];
          } 
        }

        //Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[m_visit_points.size()-1]); //Publishes "lastpoint"
        m_ship_points[i][m_visit_points.size()-1] = m_visit_points[m_visit_points.size()-1]; //Stores last point for each ship
      }
    }
     /*else if(m_assignment_method == "false"){ //East West <- Not implemented yet
      int x_coord;
      int y_coord;
  
      for(int i = 0; i < m_ship_names.size(); i++){ //Loop for Each Ship
        Notify("VISIT_POINT_"+ m_ship_names[i], m_visit_points[i]);
  
  
        
      }
    } else {
      reportRunWarning("Incomplete set of points provided ");
    }*/
  for(int i = 0; i < m_ship_names.size(); i++){
      for(int j = 0; j < m_visit_points.size(); j++){
        postViewPoint(0,0,m_ship_points[i][j],"label");
        Notify("VISIT_POINT_"+ m_ship_names[i], m_ship_points[i][j]);
      }
    }
    m_point_pub_complete = true;
  }
#endif
  trouble_iterate++;
  Notify("TROUBLE_ITERATE", trouble_iterate);
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
  //Register("NODE_REPORT",0);
  //Register("DB_TIME",0);

}


//------------------------------------------------------------
// Procedure: buildReport()

bool PointAssign::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "Points Retrieved:                           " << endl;
  for(int i = 0; i < m_visit_points.size(); i++){
    m_msgs <<"["<<i<<"]" << m_visit_points[i] << endl;
  }
  m_msgs << "total points: "<< m_visit_points.size() << endl;
  m_msgs << "trouble Mail Counter: "<< trouble_counter << endl;
  m_msgs << "trouble Iterate: "<< trouble_iterate << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Ships Detected:                             " << endl;
  for(int i = 0; i < m_ship_names.size(); i++){
    m_msgs <<"["<<i<<"]: " << m_ship_names[i] << endl;
  };
  m_msgs << "============================================" << endl;
  m_msgs << "Assignment Method: " << m_assignment_method << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Ship Assignments:                           " << endl;
  for(int i = 0; i < m_ship_names.size(); i++){
    m_msgs << "Ship: " << m_ship_names[i] << endl;
    for(int j = 0; j < m_visit_points.size(); j++){
      m_msgs << "["<<j<<"]" << m_ship_points[i][j] << endl;
    }
  }
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

/*
void PointAssign::postViewPoint(double x, double y, string label, string color){
  XYPoint point(x,y);
  point.set_label(label);
  point.set_color("vertex",color);
  point.set_param("vertex_size","4");
  Notify("VIEW_POINT",point.get_spec());
}
  */



