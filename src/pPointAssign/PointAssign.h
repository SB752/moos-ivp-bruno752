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
   void PointAssign::postViewPoint(double x, double y, string label, string color);

 private: // Configuration variables
  vector<string> m_ship_names;
  vector<string> m_visit_points;
  string m_assignment_method;
  string m_start_flag;
  string m_end_flag;

 private: // State variables
 double m_ship_reg_time;
 double m_current_time;
 bool m_first_reading;
 bool m_last_reading;
};

#endif 
