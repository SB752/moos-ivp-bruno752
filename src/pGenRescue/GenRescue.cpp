/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenRescue.cpp                                        */
/*    DATE: April 8th, 2025                             */
/*    UPDATED: April 22nd, 2025                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "GenRescue.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

GenRescue::GenRescue()
{

}

//---------------------------------------------------------
// Destructor

GenRescue::~GenRescue()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool GenRescue::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "SWIMMER_ALERT"){ 
      PointReader point;
      point.intake(msg.GetString());
      bool new_swimmer = true;

      //Tests against already registered swimmers
      for (int i=0; i<m_swimmer_points.size(); i++){
        if(point.get_id() == m_swimmer_points[i].get_id()){
          new_swimmer = false;
          break;
        }
      }
      
      if(new_swimmer){
        m_swimmer_points.push_back(point);
        m_swimmer_rescue_status.push_back(false);
        m_swimmer_rescue_score.push_back(false);
        m_field_update = true;
      }

      //Flag for first swimmer to start iterate loop
      if (!m_first_swimmer_recieved){
        m_first_swimmer_recieved = true;
      } 

    } else if(key == "FOUND_SWIMMER"){
      m_field_update = true;
      std::string s = msg.GetString();
      std::string temp = biteStringX(s, '=');
      int id = std::stoi(biteStringX(s, ','));

      for (int i=0; i<m_swimmer_points.size(); i++){
        if(id == m_swimmer_points[i].get_id()){
          m_swimmer_rescue_status[i] = true;
          break;
        }
      }

    }else if(key == "NAV_X"){
      m_x_pos = msg.GetDouble();
      m_test_name = msg.GetCommunity();
      if(!m_first_x_rec){
        m_first_x_pos = msg.GetDouble();
        m_first_x_rec = true;
      }

    } else if(key == "NAV_Y"){
      m_y_pos = msg.GetDouble();
      if(!m_first_y_rec){
        m_first_y_pos = msg.GetDouble();
        m_first_y_rec = true;
      }

    } else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool GenRescue::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenRescue::Iterate()
{
  AppCastingMOOSApp::Iterate();

  Notify("GEN_PATH_READY",m_host_community);
 
  if(m_first_swimmer_recieved){  //Doesn't run until atleast one swimmer

  //Checks if current position is within visit radius of any waypoint, marks as visited if true
  //Checked every iteration
  for(int i=0; i<m_swimmer_points.size(); i++){
    if(sqrt(pow(m_swimmer_points[i].get_x()-m_x_pos,2)+pow(m_swimmer_points[i].get_y()-m_y_pos,2)) <= visit_radius && !m_swimmer_rescue_status[i]){
      m_swimmer_rescue_status[i] = true;
      m_swimmer_rescue_score[i] = true;
    }
  }

  //Determine Fleet Score
  m_rescue_count = 0;
  for(int i=0; i<m_swimmer_points.size(); i++){
    if(m_swimmer_rescue_status[i]){
      m_rescue_count++;
    }
  }

  //Determine Ship's Score
  m_score_count = 0;
  for(int i=0; i<m_swimmer_points.size(); i++){
    if(m_swimmer_rescue_score[i]){
      m_score_count++;
    }
  }

  if(m_field_update){
    if(m_test_name == "abe"){
      findShortestPath_2(m_swimmer_points,m_swimmer_rescue_status,m_x_pos,m_y_pos);
      m_path_type = "2";
    }
    else{
      findShortestPath(m_swimmer_points,m_swimmer_rescue_status,m_x_pos,m_y_pos);
    }
    m_field_update = false;
  }
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}


//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenRescue::OnStartUp()
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
    else if(param == "path_type") {
      
      handled = true;
    }

    else if(param == "waypoint_update_var") {
      m_waypoint_update_var = value;
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

void GenRescue::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("SWIMMER_ALERT",0);
  Register("FOUND_SWIMMER",0);
  Register("NAV_X",0);
  Register("NAV_Y",0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenRescue::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "Community: " + m_host_community << endl;
  m_msgs << "Total number of swimmers: " + to_string(m_swimmer_points.size()) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Path Type: " + m_path_type << endl;
  m_msgs << "Total Swimmers Rescued: " + to_string(m_rescue_count)  << endl;
  m_msgs << "My Swimmers Rescued: " + to_string(m_score_count) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Visit Radius: " + to_string(visit_radius) << endl;
  m_msgs << "Current Pos: "+ to_string(m_x_pos) + "," + to_string(m_y_pos) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "============================================" << endl;

  return(true);
}

//------------------------------------------------------------
// Procedure: findShortestPath()

void GenRescue::findShortestPath(vector<PointReader> points, vector<bool> visit_status, double x, double y)
{

  //Checks if all points have been visited, if so, sends message to return to start
  
  if(m_rescue_count == points.size()){
    Notify(m_waypoint_update_var,"points = "+to_string(m_first_x_pos)+","+to_string(m_first_y_pos));
    return;
  }

  //Some points still need to be visited, starts pathfinding
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
  }
  //Sends waypoint list once all points proccessed
  waypoints_output += waypoint_list.get_spec();
  Notify(m_waypoint_update_var,waypoints_output);
}

//------------------------------------------------------------
// Procedure: findShortestPath_2()

void GenRescue::findShortestPath_2(vector<PointReader> points, vector<bool> visit_status, double x, double y)
{
//This function works almost the same as findShortestPath, except it selects the next waypoint based on the next two closest points
  //Checks if all points have been visited, if so, sends message to return to start
  
  if(m_rescue_count == points.size()){
    Notify(m_waypoint_update_var,"points = "+to_string(m_first_x_pos)+","+to_string(m_first_y_pos));
    return;
  }

  //Some points still need to be visited, starts pathfinding
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
    int min_index = -1;
    for(int i=0; i<points_working_copy.size(); i++){ //finds closest point
      double dist_1 = sqrt(pow(points_working_copy[i].get_x()-Prev_x,2)+pow(points_working_copy[i].get_y()-Prev_y,2));

      for(int j=0; j<points_working_copy.size(); j++){
        if(i == j){
          continue;
        }
        double dist_2 = sqrt(pow(points_working_copy[j].get_x()-points_working_copy[i].get_x(),2)+pow(points_working_copy[j].get_y()-points_working_copy[i].get_y(),2));
        if(dist_1 + dist_2 <= min_dist){
          min_dist = dist_1 + dist_2;
          min_index = i;
      }

      }
      if(min_index = -1) //If only one point left, set min_index to that point
      min_index = 0;
    }
    
    waypoint_list.add_vertex(points_working_copy[min_index].get_x(),points_working_copy[min_index].get_y());
    Prev_x = points_working_copy[min_index].get_x(); //Resets previous point to closest point for next loop iteration
    Prev_y = points_working_copy[min_index].get_y();
    points_working_copy.erase(points_working_copy.begin()+min_index); //removes closest point from list for remaining iterations
}
  //Sends waypoint list once all points proccessed
  waypoints_output += waypoint_list.get_spec();
  Notify(m_waypoint_update_var,waypoints_output);

}

//------------------------------------------------------------
// Procedure: findShortestPath_3()
void GenRescue::findShortestPath_3(vector<PointReader> points, vector<bool> visit_status, double x, double y)
{
//This function works almost the same as findShortestPath, except it selects the next waypoint based on the next two closest points
  //Checks if all points have been visited, if so, sends message to return to start
  
  if(m_rescue_count == points.size()){
    Notify(m_waypoint_update_var,"points = "+to_string(m_first_x_pos)+","+to_string(m_first_y_pos));
    return;
  }

  //Some points still need to be visited, starts pathfinding
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
    int min_index = -1;
    for(int i=0; i<points_working_copy.size(); i++){ //finds closest point
      double dist_1 = sqrt(pow(points_working_copy[i].get_x()-Prev_x,2)+pow(points_working_copy[i].get_y()-Prev_y,2));

      for(int j=0; j<points_working_copy.size(); j++){
        if(i == j){
          continue;
        }
        double dist_2 = sqrt(pow(points_working_copy[j].get_x()-points_working_copy[i].get_x(),2)+pow(points_working_copy[j].get_y()-points_working_copy[i].get_y(),2));

        for(int k=0; k<points_working_copy.size(); k++){
          if(i == k || j == k){
            continue;
          }
          double dist_3 = sqrt(pow(points_working_copy[k].get_x()-points_working_copy[j].get_x(),2)+pow(points_working_copy[k].get_y()-points_working_copy[j].get_y(),2));
          if(dist_1 + dist_2 + dist_3 <= min_dist){
            min_dist = dist_1 + dist_2 + dist_3;
            min_index = i;
        }

      }

    }

    if(min_index = -1) //If only one point left, set min_index to that point
    min_index = 0;
    
    waypoint_list.add_vertex(points_working_copy[min_index].get_x(),points_working_copy[min_index].get_y());
    Prev_x = points_working_copy[min_index].get_x(); //Resets previous point to closest point for next loop iteration
    Prev_y = points_working_copy[min_index].get_y();
    points_working_copy.erase(points_working_copy.begin()+min_index); //removes closest point from list for remaining iterations
}
  //Sends waypoint list once all points proccessed
  waypoints_output += waypoint_list.get_spec();
  Notify(m_waypoint_update_var,waypoints_output);

}
}
