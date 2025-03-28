/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.h                                          */
/*    DATE: March 7th, 2025                             */
/*    UPDATED: March 18th, 2025*/
/************************************************************/

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include"../pPointAssign/PointReader.h"
#include "MBUtils.h"
#include "XYSegList.h"

class GenPath : public AppCastingMOOSApp
{
 public:
   GenPath();
   ~GenPath();

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
   void publishWaypoints();

 private: // Configuration variables
   std::vector<PointReader> m_mission_points;
   std::vector<PointReader> m_waypoints_list;

   std::string m_start_flag;
   std::string m_end_flag;

 private: // State variables
   bool m_first_reading;
   bool m_last_reading;

   bool m_first_x;
   bool m_first_y;

   double m_x_pos;
   double m_y_pos;

   bool m_waypoints_published;

   double m_score_count;
   int m_trouble_tracker;

   bool m_start_assignment;

   double Prev_x =  0.0;
   double Prev_y = 0.0;

   XYSegList waypoint_list;

   std::vector<bool> m_visited_points_tracker;

   double visit_radius;
   bool m_regen_path;

   int m_wpt_index;

   bool recalc_at_fueling;


};

#endif 
