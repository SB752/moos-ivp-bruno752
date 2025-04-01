/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ZigLeg.h                                      */
/*    DATE: 01APR2025                                        */
/************************************************************/

#ifndef ZigLeg_HEADER
#define ZigLeg_HEADER

#include <string>
#include "IvPBehavior.h"
#include "XYRangePulse.h"

class BHV_ZigLeg : public IvPBehavior {
public:
  BHV_ZigLeg(IvPDomain);
  ~BHV_ZigLeg() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
  IvPFunction* buildFunctionWithZAIC();

protected: // Configuration parameters
double m_zig_angle=45;
double m_zig_duration=10;

protected: // State variables
double m_current_index=0;
double m_previous_index=0;
double m_current_time;
bool m_zig_ready=false;
double m_zig_start_time=0;
double m_zig_delay=5;
double m_osx;
double m_osy;

double m_nav_heading;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_ZigLeg(domain);}
}
#endif
