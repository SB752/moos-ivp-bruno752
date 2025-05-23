/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenRescue.cpp                                        */
/*    DATE: April 8th, 2025                             */
/*    UPDATED: April 29th, 2025                             */
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

    if(key == "SWIMMER_ALERT" && (msg.GetCommunity() != m_enemy_rescuer && msg.GetCommunity() != m_enemy_scout)){ 
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

    } else if(key == "FOUND_SWIMMER" && (msg.GetCommunity() != m_enemy_rescuer && msg.GetCommunity() != m_enemy_scout)){
      m_field_update = true; //Always true
      //Intakes found swimmer message data as a point
      PointReader found_point;
      found_point.intake(msg.GetString());
      if(found_point.get_finder() != m_my_name  &&!m_enemy_rescuer_identified){
        m_enemy_rescuer = found_point.get_finder();  //ID Enemy Rescuer
        m_enemy_rescuer_identified = true;
      }

      //Tests against already registered swimmers
      //If new found, sets status to true
      bool new_swimmer = true;
      for (int i=0; i<m_swimmer_points.size(); i++){
        if(found_point.get_id() == m_swimmer_points[i].get_id()){
          new_swimmer = false;
          m_swimmer_rescue_status[i] = true;
          break;
        }
      }
      
      //if new swimmer, add to list
      //including rescue status as "true"
      if(new_swimmer){
        m_swimmer_points.push_back(found_point);
        m_swimmer_rescue_status.push_back(true);
        m_swimmer_rescue_score.push_back(false);
      }

    } else if(key == "NAV_X"){
      m_x_pos = msg.GetDouble();
      m_my_name = msg.GetCommunity();
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

      //
    } else if(key == "NAV_HEADING"){
      m_heading = msg.GetDouble();
    } else if (key == "NODE_REPORT_LOCAL"){
      string report = msg.GetString();
      NodeRecord new_node_record = string2NodeRecord(report);
      m_team_color = new_node_record.getColor();  //Find my team color

    } else if (key == "NODE_REPORT"){
      string report = msg.GetString();
      NodeRecord new_node_record = string2NodeRecord(report);
      string color = new_node_record.getColor();
      if (color == m_team_color){
        m_teammate_name = new_node_record.getName();  //Find my teammate
      } else if(new_node_record.getName() == m_enemy_rescuer){
        m_enemy_res_x_pos = new_node_record.getX();
        m_enemy_res_y_pos = new_node_record.getY();
        m_enemy_res_heading = new_node_record.getHeading();
        m_enemy_res_speed = new_node_record.getSpeed();
      } else {
        m_enemy_scout = new_node_record.getName();  //Find enemy scout
        m_enemy_scout_x_pos = new_node_record.getX();
        m_enemy_scout_y_pos = new_node_record.getY();
        m_enemy_scout_heading = new_node_record.getHeading();
      }

    } else if (key == "SURVEY_UPDATE"){
      if (msg.GetCommunity() != m_my_name){   //Defense against enemy spoofing
        m_cyber_under_attack = true;
        Notify(m_waypoint_update_var,m_waypoints_output);
      }

    } else if (key == "RETURN"){
      if (msg.GetCommunity() != m_my_name && msg.GetCommunity() != "shoreside"){
        m_cyber_under_attack = true;
        //Notify("RETURN","false");
        //Notify("SURVEY","true");
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
 
  if(m_first_swimmer_recieved && !m_cyber_under_attack){  //Doesn't run until atleast one swimmer
  //Checks if current position is within visit radius of any waypoint, marks as visited if true
  //Checked every iteration
  /*
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
    */

    /*
  //Determine Swimmer Centroid
  m_swimmer_centroid = findSwimmerCentroid(m_swimmer_points,m_swimmer_rescue_status);
  */

    if(count(m_swimmer_rescue_status.begin(),m_swimmer_rescue_status.end(),true) == m_swimmer_points.size()){
    m_enemy_scout_turned = true;
    Notify(m_waypoint_update_var,"points = "+to_string(m_enemy_scout_x_pos)+","+to_string(m_enemy_scout_y_pos));
  }

  if(m_field_update){
    //findPath_Centroid(m_swimmer_points,m_swimmer_rescue_status,m_x_pos,m_y_pos);
    findShortestPath_V2(m_swimmer_points,m_swimmer_rescue_status,m_x_pos,m_y_pos);
    m_field_update = false;
  }
  }
    
  /*
    //Find location 5 feet in front of enemy rescuer
    double enemy_x = m_enemy_res_x_pos;
    double enemy_y = m_enemy_res_y_pos;
    double enemy_heading = m_enemy_res_heading;
    double enemy_x_offset = enemy_x + 20*cos(degToRad(enemy_heading));
    double enemy_y_offset = enemy_y + 20*sin(degToRad(enemy_heading));
    //Post location to MOOSDB
  
    std::string targ_mes = "x=" + to_string(enemy_x_offset) + ",y=" + to_string(enemy_y_offset);
    NodeMessage targ_mes_node;
    targ_mes_node.setSourceNode(m_my_name);
    targ_mes_node.setDestNode(m_teammate_name);
    targ_mes_node.setVarName("ENEMY_RES_LOC");
    targ_mes_node.setStringVal(targ_mes);
    Notify("NODE_MESSAGE_LOCAL",targ_mes_node.getSpec());

    */


  //Send spoof message to enemy
  if(m_cyber_attack){
    NodeMessage cyberwar_node_targ1;
    cyberwar_node_targ1.setSourceNode(m_enemy_scout);  
    cyberwar_node_targ1.setDestNode(m_enemy_rescuer);
    cyberwar_node_targ1.setVarName("SURVEY_UPDATE");
    cyberwar_node_targ1.setStringVal("points = " + to_string(100) + "," + to_string(-15));
    Notify("NODE_MESSAGE_LOCAL",cyberwar_node_targ1.getSpec());


    NodeMessage cyberwar_node_targ2;
    cyberwar_node_targ2.setSourceNode(m_enemy_rescuer);  
    cyberwar_node_targ2.setDestNode(m_enemy_scout);
    cyberwar_node_targ2.setVarName("RETURN");
    if(!m_enemy_scout_turned){
      cyberwar_node_targ2.setStringVal("true");
    } else {
      cyberwar_node_targ2.setStringVal("false");
      Notify("NODE_MESSAGE_LOCAL",cyberwar_node_targ2.getSpec());
      cyberwar_node_targ2.setVarName("STATION_KEEP");

    }
    Notify("NODE_MESSAGE_LOCAL",cyberwar_node_targ2.getSpec());

    //cyberwar_node_targ2.setVarName("SURVEY");
    //cyberwar_node_targ2.setStringVal("true");
    //Notify("NODE_MESSAGE_LOCAL",cyberwar_node_targ2.getSpec());

    
    
  }

  if(m_cyber_under_attack){
    Notify(m_waypoint_update_var,m_waypoints_output);
    Notify("CYBER_DEFENSE_ACTIVATED","true");
    return(true);
    //m_cyber_under_attack = false;
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

    else if (param == "cyber_attack"){
      if(value == "false"){
        m_cyber_attack = false;
      }
      handled = true;
    }

    else if (param == "cyber_defense"){
      if(value == "false"){
        m_cyber_defense = false;
      }
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

void GenRescue::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("SWIMMER_ALERT",0);
  Register("FOUND_SWIMMER",0);
  Register("NAV_X",0);
  Register("NAV_Y",0);
  Register("NAV_HEADING",0);
  Register("NODE_REPORT",0);
  Register("NODE_REPORT_LOCAL",0);
  if(m_cyber_defense){
    Register("SURVEY_UPDATE",0);
    //Register("RETURN",0);
  }
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenRescue::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "Community: " + m_host_community << endl;
  m_msgs << "Total number of swimmers: " + to_string(m_swimmer_points.size()) << endl;
  m_msgs << "============================================" << endl;
  //m_msgs << "Swimmer Centroid: " + doubleToString(m_swimmer_centroid[0]) + "," + doubleToString(m_swimmer_centroid[1]) << endl;
  //m_msgs << "Total Swimmers Rescued: " + to_string(m_rescue_count)  << endl;
  //m_msgs << "My Swimmers Rescued: " + to_string(m_score_count) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "My Team Color: " + m_team_color << endl;
  m_msgs << "My Teammate: " + m_teammate_name << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Enemy Rescuer: " + m_enemy_rescuer << endl;
  m_msgs << "Enemy Rescuer Pos: " + to_string(m_enemy_res_x_pos) + "," + to_string(m_enemy_res_y_pos) << endl;
  m_msgs << "Enemy Rescuer Heading: " + to_string(m_enemy_res_heading) << endl;
  m_msgs << "Enemy Rescuer Speed: " + to_string(m_enemy_res_speed) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Enemy Scout: " + m_enemy_scout << endl;
  m_msgs << "Enemy Scout Pos: " + to_string(m_enemy_scout_x_pos) + "," + to_string(m_enemy_scout_y_pos) << endl;
  m_msgs << "Enemy Scout Heading: " + to_string(m_enemy_scout_heading) << endl;
  m_msgs << "Enemy Scout Turned: " + to_string(m_enemy_scout_turned) << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Visit Radius: " + to_string(visit_radius) << endl;
  m_msgs << "Current Pos: "+ to_string(m_x_pos) + "," + to_string(m_y_pos) << endl;

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
  m_waypoints_output = "points = "; //reset output string
  
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

  //Finds point closest to enemy rescuer
  double enemy_min_dist = 1000000;
  int enemy_index = -1;
  for(int i=0; i<points_working_copy.size(); i++){
    double enemy_dist = sqrt(pow(points_working_copy[i].get_x()-m_enemy_res_x_pos,2)+pow(points_working_copy[i].get_y()-m_enemy_res_y_pos,2));
    if(enemy_dist <= enemy_min_dist){
      enemy_min_dist = enemy_dist;
      enemy_index = i;
    }
  }

  //if enemy rescuer closer than me to their closest point, remove point from the list
  if(enemy_min_dist <= sqrt(pow(points_working_copy[enemy_index].get_x()-Prev_x,2)+pow(points_working_copy[enemy_index].get_y()-Prev_y,2))){
    points_working_copy.erase(points_working_copy.begin()+enemy_index);
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
  m_waypoints_output += waypoint_list.get_spec();
  Notify(m_waypoint_update_var,m_waypoints_output);
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

//------------------------------------------------------------
// Procedure: findPath_Centroid()

void GenRescue::findPath_Centroid(vector<PointReader> points, vector<bool> visit_status, double x, double y)
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

    //Finds point closest to enemy rescuer
    double enemy_min_dist = 1000000;
    int enemy_index = -1;
    for(int i=0; i<points_working_copy.size(); i++){
      double enemy_dist = sqrt(pow(points_working_copy[i].get_x()-m_enemy_res_x_pos,2)+pow(points_working_copy[i].get_y()-m_enemy_res_y_pos,2));
      if(enemy_dist <= enemy_min_dist){
        enemy_min_dist = enemy_dist;
        enemy_index = i;
      }
    }
  
    //if enemy rescuer closer than me to their closest point, remove point from the list
    if(enemy_min_dist <= sqrt(pow(points_working_copy[enemy_index].get_x()-Prev_x,2)+pow(points_working_copy[enemy_index].get_y()-Prev_y,2))){
      points_working_copy.erase(points_working_copy.begin()+enemy_index);
    }
      
  //Sorts points by closest distance to previous point, starting with first point
  //Only runs once per function call
  while(!points_working_copy.empty()){
    double min_dist = 1000000;
    int min_index = 0;
    for(int i=0; i<points_working_copy.size(); i++){ //finds closest point
      double dist_ship2pt = sqrt(pow(points_working_copy[i].get_x()-Prev_x,2)+pow(points_working_copy[i].get_y()-Prev_y,2));
      double dist_pt2cent = sqrt(pow(points_working_copy[i].get_x()-m_swimmer_centroid[0],2)+pow(points_working_copy[i].get_y()-m_swimmer_centroid[1],2));
      if(dist_ship2pt + dist_pt2cent <= min_dist){
        min_dist = dist_ship2pt + dist_pt2cent;
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
// Procedure: findShortestPath_V2()

void GenRescue::findShortestPath_V2(vector<PointReader> points, vector<bool> visit_status, double x, double y)
{

  //Checks if all points have been visited, if so, sends message to return to start
  if(count(visit_status.begin(),visit_status.end(),true) == points.size()){
    m_enemy_scout_turned = true;
    Notify(m_waypoint_update_var,"points = "+to_string(m_enemy_scout_x_pos)+","+to_string(m_enemy_scout_y_pos));
    return;
  }

  //Some points still need to be visited, starts pathfinding
  double Prev_x = x;
  double Prev_y = y;
  double bearing_0 = degToRad(m_heading);

  XYSegList waypoint_list;
  vector<PointReader> points_working_copy;
  vector<bool> visit_status_working_copy;
  m_waypoints_output = "points = "; //reset output string
  
  //sets intial points for pathfinding, generates bool vector for tracking visit
  //Only runs once
  points_working_copy = points;
  visit_status_working_copy = visit_status;

  //Removes points that have been visited from working copy
  for(int i=0; i<points_working_copy.size(); i++){
    if(visit_status_working_copy[i]){
      visit_status_working_copy.erase(visit_status_working_copy.begin()+i);
      points_working_copy.erase(points_working_copy.begin()+i);
      --i;
    }
  }

  //Finds point closest to enemy rescuer, if not stopped
  if(m_enemy_res_speed != 0){
  double enemy_min_dist = 1000000;
  double enemy_dist     = 1000000;
  int enemy_index = 0;
  for(int i=0; i<points_working_copy.size(); i++){
    enemy_dist = sqrt(pow(points_working_copy[i].get_x()-m_enemy_res_x_pos,2)+pow(points_working_copy[i].get_y()-m_enemy_res_y_pos,2));
    if(enemy_dist <= enemy_min_dist){
      enemy_min_dist = enemy_dist;
      enemy_index = i;
    }
  }

  //if enemy rescuer closer than me to their closest point, remove point from the list
  if(enemy_min_dist <= sqrt(pow(points_working_copy[enemy_index].get_x()-Prev_x,2)+pow(points_working_copy[enemy_index].get_y()-Prev_y,2))){
    points_working_copy.erase(points_working_copy.begin()+enemy_index);
  }
}
      
  //Sorts points by closest distance to previous point, starting with first point
  //Only runs once per function call
  while(!points_working_copy.empty()){
    double min_dist = 3000001;
    double dist_1   = 1000000;
    double dist_2   = 1000000;
    double dist_3   = 1000000;
    int min_index   = 0;

    double bearing_1 = 0;
    double bearing_2 = 0;
    double bearing_3 = 0;
    for(int i=0; i<points_working_copy.size(); i++){ //finds closest point
      if(points_working_copy.size() == 1){   //Shortcuts loop if only one point left
        min_index = i;
        break;
      }
      dist_1 = sqrt(pow(points_working_copy[i].get_x()-Prev_x,2)+pow(points_working_copy[i].get_y()-Prev_y,2));
      bearing_1 = 90 - atan2(points_working_copy[i].get_x()-Prev_x,points_working_copy[i].get_y()-Prev_y);
      dist_1 = dist_1 + m_turn_radius *abs(bearing_0 - bearing_1);
      for(int j=0; j<points_working_copy.size(); j++){ //finds closest point
        if(points_working_copy.size() == 2){  //shortcuts loop if only two points left
          min_index = i;
          break;
        }
        if(i == j){ //Prevents from measuring the same points repeatedly
          continue;
        }

        dist_2 = sqrt(pow(points_working_copy[j].get_x()-points_working_copy[i].get_x(),2)+pow(points_working_copy[j].get_y()-points_working_copy[i].get_y(),2));
        bearing_2 = 90 - atan2(points_working_copy[j].get_x()-points_working_copy[i].get_x(),points_working_copy[j].get_y()-points_working_copy[i].get_y());
        dist_2 = dist_2 + m_turn_radius *abs(bearing_1 - bearing_2);
        for(int k=0; k<points_working_copy.size(); k++){ //finds closest point
          if(points_working_copy.size() == 3){  //shortcuts loop if only three points left
            min_index = i;
            break;
          }
          if(i == k || j == k){ //Prevents from measuring the same points repeatedly
            continue;
          }

          dist_3 = sqrt(pow(points_working_copy[k].get_x()-points_working_copy[j].get_x(),2)+pow(points_working_copy[k].get_y()-points_working_copy[j].get_y(),2));
          bearing_3 =  90 - atan2(points_working_copy[k].get_x()-points_working_copy[j].get_x(),points_working_copy[k].get_y()-points_working_copy[j].get_y());
          dist_3 = dist_3 + m_turn_radius *abs(bearing_2 - bearing_3);
          if(dist_1 + dist_2 + dist_3 <= min_dist){
            min_dist = dist_1 + dist_2 + dist_3;
            min_index = i;
          }
        }
      }
    }
    waypoint_list.add_vertex(points_working_copy[min_index].get_x(),points_working_copy[min_index].get_y());
    //Resets previous point to closest point for next loop iteration
    bearing_0 = atan2(points_working_copy[min_index].get_x()-Prev_x,points_working_copy[min_index].get_y()-Prev_y);
    Prev_x = points_working_copy[min_index].get_x(); 
    Prev_y = points_working_copy[min_index].get_y();
    points_working_copy.erase(points_working_copy.begin()+min_index); //removes closest point from list for remaining iterations
  }
  //Sends waypoint list once all points proccessed
  m_waypoints_output += waypoint_list.get_spec();
  Notify(m_waypoint_update_var,m_waypoints_output);
}




//------------------------------------------------------------
// Procedure: findPath_Centroid_2()


//------------------------------------------------------------
// Procedure: findSwimmerCentroid()
std::vector<double> GenRescue::findSwimmerCentroid(std::vector<PointReader> points, std::vector<bool> visit_status)
{
  //Finds the centroid of all swimmers
  double x_sum = 0;
  double y_sum = 0;
  int count = 0;

  for(int i=0; i<points.size(); i++){
    if(!visit_status[i]){
      x_sum += points[i].get_x();
      y_sum += points[i].get_y();
      count++;
    }
  }

  vector<double> centroid = {0,0};
  if (count == 0){
    return centroid;
  }  else{
  centroid[0] = (x_sum/count);
  centroid[1] = (y_sum/count);
  return centroid;
  };


}

double GenRescue::degToRad(double degrees) {
  return degrees * M_PI / 180.0;
}