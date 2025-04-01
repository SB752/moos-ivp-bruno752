/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pulse.h                                      */
/*    DATE: 01APR2025                                        */
/************************************************************/

#ifndef Pulse_HEADER
#define Pulse_HEADER

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
XYRangePulse m_pulse;

protected: // Configuration parameters
double m_range=5;
double m_duration=10;

protected: // State variables
double m_current_index=0;
double m_previous_index=0;
double m_current_time;
bool m_pulse_ready=false;
double m_pulse_time=0;
double m_pulse_delay=0;
double m_osx;
double m_osy;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_ZigLeg(domain);}
}
#endif
