/************************************************************/
/*    NAME: StephenBruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef Odometry_HEADER
#define Odometry_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Odometry : public AppCastingMOOSApp
{
 public:
  Odometry();
  ~Odometry();

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
  bool m_first_reading;
  std::string m_Xtarget;
  std::string m_Ytarget;



 private: // State variables
  double m_current_x;
  double m_current_y;
  double m_previous_x;
  double m_previous_y;
  double m_total_distance;
  double m_current_time;
  double m_previous_time;
};

#endif 
