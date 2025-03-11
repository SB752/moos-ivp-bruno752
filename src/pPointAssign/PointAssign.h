/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                    */
/*    DATE: March 4th, 2025                                  */
/************************************************************/

#ifndef PointAssign_HEADER
#define PointAssign_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include "PointReader.h"

class PointAssign : public AppCastingMOOSApp
{
 public:
   PointAssign();
   ~PointAssign();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
   void postViewPoint(double x, double y, std::string label, std::string color);

 private: // Configuration variables
  std::vector<std::string> m_ship_names;
  std::vector<PointReader> m_visit_points;
  std::vector<PointReader> m_ship_points_A;
  std::vector<PointReader> m_ship_points_B;

  std::string m_assignment_method;
  std::string m_start_flag;
  std::string m_end_flag;

 private: // State variables
 double m_ship_reg_time;
 double m_current_time;
 bool m_first_reading;
 bool m_last_reading;

 bool m_sort_complete;
 bool m_point_pub_complete;

 int m_export_count;

 bool m_ship_A_ready;
 bool m_ship_B_ready;
};

#endif 
