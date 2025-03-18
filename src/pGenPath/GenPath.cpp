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

  m_first_x = false;
  m_first_y = false;

  m_waypoints_published = false;

  m_score_count = 0;
  m_trouble_tracker =1;

  m_start_assignment = false;

  visit_radius = 5;

  m_regen_path = false;

  m_wpt_index = 0;

  //Prev_x;
  //Prev_y;

  //waypoint_list;

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
       } else if (msg.GetString() == m_end_flag && !m_last_reading){
         m_last_reading = true;
         
       } else {
         PointReader point;
         point.intake(msg.GetString());
         m_mission_points.push_back(point);
       }

     } else if(key == "NAV_X"){
       m_x_pos = msg.GetDouble();
       m_first_x = true;

     } else if(key == "NAV_Y"){
       m_y_pos = msg.GetDouble();
      m_first_y = true;

     } else if(key == "PATH_INDEX"){
        m_wpt_index = static_cast<int>(msg.GetDouble());

     }

      else if(key == "GENPATH_REGENERATE"){
        if(msg.GetString() == "true")
          m_regen_path = true;


     } else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
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

  Notify("GEN_PATH_READY",m_host_community);
 
  if(m_last_reading && !m_waypoints_published){  //Doesn't run until all points are collected

    //Initializes vector for tracking visit status of waypoints, all start false
    for(int i=0; i<m_mission_points.size(); i++){
      m_visited_points_tracker.push_back(false);
    }

    findShortestPath(m_mission_points,m_visited_points_tracker,m_x_pos,m_y_pos);
    m_waypoints_published = true;
  }

  //Won't work because waypoint index different than mission_points index
  /*
  if(m_waypoints_published){
    if(sqrt(pow(m_mission_points[m_wpt_index].get_x()-m_x_pos,2)+pow(m_mission_points[m_wpt_index].get_y()-m_y_pos,2)) <= visit_radius && !m_visited_points_tracker[m_wpt_index]){
     m_visited_points_tracker[m_wpt_index] = true;
     m_score_count++;
  }
}
  */

  //Checks if current position is within visit radius of any waypoint, marks as visited if true
  //Checked every iteration
  for(int i=0; i<m_mission_points.size(); i++){
    if(sqrt(pow(m_mission_points[i].get_x()-m_x_pos,2)+pow(m_mission_points[i].get_y()-m_y_pos,2)) <= visit_radius && !m_visited_points_tracker[i]){
      m_visited_points_tracker[i] = true;
      m_score_count++;
    }
  }



  
  if(m_regen_path){
    if(m_score_count == m_mission_points.size()){
      Notify("RETURN","true");
    } else{
    findShortestPath(m_mission_points,m_visited_points_tracker,m_x_pos,m_y_pos);
    m_regen_path = false;
  }
  Notify("REGEN_PATH","false");
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
    if(param == "visit_radius") {
      visit_radius = stod(value);
      handled = true;
    }
    else if(param == "recalc_at_fueling") {
      if(value == "true")
        Notify("FUEL_REM_BHV","endflag=REGEN_PATH=true");
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();
  Notify("GEN_PATH_READY",m_host_community);
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
  Register("PATH_INDEX",0);
  Register("GENPATH_REGENERATE",0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "Community: " + m_host_community << endl;
  m_msgs << "First Point Recieved: " + to_string(m_first_reading) << endl;
  m_msgs << "Last Point Recieved: " + to_string(m_last_reading) << endl;
  m_msgs << "NAV X/Y Recieved: " + to_string(m_first_x) + "/" + to_string(m_first_y) << endl;
  m_msgs << "Number of Assigned waypoints: " + to_string(m_mission_points.size()) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Current Waypoint Index: "+ to_string(m_wpt_index) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Visit Radius: " + to_string(visit_radius) << endl;
  m_msgs << "Current Pos: "+ to_string(m_x_pos) + "," + to_string(m_y_pos) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Waypoints Visited:                     " << endl;
  m_msgs << to_string(m_score_count)                        << endl;
  m_msgs << "============================================" << endl;

  return(true);
}

//------------------------------------------------------------
// Procedure: findShortestPath()

void GenPath::findShortestPath(vector<PointReader> points, vector<bool> visit_status, double x, double y)
{
  if(m_score_count == points.size()){
    Notify("UPDATES","points = $(START_POS)");
    return;
  }


  double Prev_x = 0.0;
  double Prev_y = 0.0;
  XYSegList waypoint_list;
  vector<PointReader> points_working_copy;
  vector<bool> visit_status_working_copy;
  string waypoints_output = "points = ";
  
  //sets intial points for pathfinding, generates bool vector for tracking visit
  //Only runs once
  points_working_copy = points;
  visit_status_working_copy = visit_status;

  Prev_x = x;
  Prev_y = y;

  //Removes points that have been visited from working copy
  for(int i=0; i<points_working_copy.size(); i++){
    if(visit_status_working_copy[i]){
      visit_status_working_copy.erase(visit_status_working_copy.begin()+i);
      points_working_copy.erase(points_working_copy.begin()+i);
      --i;
    }
  }
      
  //Sorts points by closest distance to previous point, starting with first point
  //Only runs once per function call
  while(!points_working_copy.empty()){
    double min_dist = 1000000;
    int min_index = 0;
    for(int i=0; i<points_working_copy.size(); i++){ //finds closest point
      double dist = sqrt(pow(points_working_copy[i].get_x()-Prev_x,2)+pow(points_working_copy[i].get_y()-Prev_y,2));
      if(dist <= min_dist){
        min_dist = dist;
        min_index = i;
      }
    }
    waypoint_list.add_vertex(points_working_copy[min_index].get_x(),points_working_copy[min_index].get_y());
    Prev_x = points_working_copy[min_index].get_x(); //Resets previous point to closest point for next loop iteration
    Prev_y = points_working_copy[min_index].get_y();
    points_working_copy.erase(points_working_copy.begin()+min_index); //removes closest point from list for remaining iterations
    Notify("LAST_UPDATE",waypoint_list.get_spec());
  }
  //Sends waypoint list once all points proccessed
  waypoints_output += waypoint_list.get_spec();
  Notify("UPDATES",waypoints_output);
  


}



