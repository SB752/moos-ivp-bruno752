/************************************************************/
/*    NAME: Stephen Bruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include"../pPointAssign/PointReader.h"
#include "MBUtils.h"

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

 private: // Configuration variables
   std::vector<PointReader> m_mission_points;
   std::vector<PointReader> m_waypoints_list;

   std::string m_waypoints_output;

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


};

#endif 
