/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenRescue.h                                          */
/*    DATE: April 8th, 2025                             */
/*    UPDATED: April 29th, 2025    */
/************************************************************/

#ifndef GenRescue_HEADER
#define GenRescue_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include"../pPointAssign/PointReader.h"
#include "MBUtils.h"
#include "XYSegList.h"
#include "NodeRecord.h"
#include "NodeRecordUtils.h"
#include "NodeMessage.h"
#include "UFieldUtils.h"
#include <cmath>

class GenRescue : public AppCastingMOOSApp
{
 public:
   GenRescue();
   ~GenRescue();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

   void findShortestPath(std::vector<PointReader> points, std::vector<bool> visit_status, double x, double y);
   void findShortestPath_2(std::vector<PointReader> points, std::vector<bool> visit_status, double x, double y);
   void findShortestPath_3(std::vector<PointReader> points, std::vector<bool> visit_status, double x, double y);
   void findShortestPath_V2(std::vector<PointReader> points, std::vector<bool> visit_status, double x, double y);
   void findPath_Centroid(std::vector<PointReader> points, std::vector<bool> visit_status, double x, double y);
   std::vector<double> findSwimmerCentroid(std::vector<PointReader> points, std::vector<bool> visit_status);
   double degToRad(double degrees);

 private: // Configuration variables
 std::string m_waypoint_update_var = "SURVEY_UPDATE";
 std::string m_my_name = "";
 std::string m_team_color = "";
 std::string m_teammate_name = "";
 std::string m_enemy_rescuer = "";
 std::string m_enemy_scout = "";
 std::string m_waypoints_output = "points = ";

 //Special Features
 bool m_cyber_attack = true;
 bool m_cyber_defense = true;
 bool m_cyber_under_attack = false;

 double m_turn_radius = 3;
  

 private: // State variables
 //Key Vectors
  std::vector<PointReader> m_swimmer_points; //Master list of Points
  std::vector<bool> m_swimmer_rescue_status; //List of points rescued by anyone
  std::vector<bool> m_swimmer_rescue_score; //List of points visited by this vehicle
  std::vector<bool> m_swimmer_conceded; //List of points conceded by this vehicle
  std::vector<double> m_swimmer_value; //List of values for each swimmer
  std::vector<double> m_swimmer_centroid = {0,0}; //X and Y value for centroid of all unrescued swimmers

  double m_x_pos;
  double m_y_pos;
  double m_heading;

  double m_rescue_count;
  double m_score_count;

  double Prev_x =  0.0;
  double Prev_y = 0.0;

  XYSegList waypoint_list;

  double visit_radius = 5;

  bool m_first_swimmer_recieved = false;
  bool m_field_update = false;

  bool m_first_x_rec = false;
  bool m_first_y_rec = false;

  double m_first_x_pos;
  double m_first_y_pos;

  std::string m_path_type = "1";

  //Other Team Variables
  bool m_enemy_rescuer_identified = false;
  double m_enemy_res_x_pos;
  double m_enemy_res_y_pos;
  double m_enemy_res_heading;
  double m_enemy_res_speed;

  double m_enemy_scout_x_pos;
  double m_enemy_scout_y_pos;
  double m_enemy_scout_heading;
  bool m_enemy_scout_turned = false;

};

#endif 
