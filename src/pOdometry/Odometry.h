/************************************************************/
/*    NAME: StephenBruno                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.h                                          */
/*    DATE: February 13th, 2025                             */
/*    UPDATED: February 25th, 2025                           */
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
  double m_depth_thresh;
  std::string m_odometer_units;
  double m_errorcheck_time;
  double m_objective_dist;

 private: // State variables
  double m_current_x;
  double m_current_y;
  double m_previous_x;
  double m_previous_y;
  double m_total_distance;
  double m_current_time;
  double m_lastupdate_time;
  double m_current_depth;
  double m_dist_at_depth;
};

#endif 
