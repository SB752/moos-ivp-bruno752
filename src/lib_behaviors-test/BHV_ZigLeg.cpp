/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ZigLeg.cpp                                    */
/*    DATE: 01APR2025                                        */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Pulse.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Pulse::BHV_Pulse(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y");
  addInfoVars("WPT_INDEX","no_warning");



}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Pulse::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "pulse_range") && isNumber(val)) {
    // Set local member variables here
    m_range = double_val;
    return(true);
  }
  else if((param=="pulse_duration") && isNumber(val)) {
    m_duration = double_val;
    m_pulse.set_duration(m_duration);
    return(true);
  }
  else if (param == "bar") {
    // return(setBooleanOnString(m_my_bool, val));
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Pulse::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Pulse::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Pulse::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Pulse::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Pulse::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Pulse::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Pulse::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Pulse::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;

  m_current_index = getBufferDoubleVal("WPT_INDEX");
  m_current_time = getBufferCurrTime();
  m_osx = getBufferDoubleVal("NAV_X");
  m_osy = getBufferDoubleVal("NAV_Y");


  //Build non-changing portions of pulse spec
  m_pulse.set_label("BHV_Pulse");
  m_pulse.set_rad(m_range);
  m_pulse.set_duration(m_duration);
  m_pulse.set_color("edge", "yellow");
  m_pulse.set_color("fill", "yellow");


  if (m_current_index != m_previous_index) { //Get Pulse Ready
    m_pulse_ready = true;
    m_previous_index = m_current_index;
    m_pulse_time = m_current_time + 5;
  }

  if(m_pulse_ready && (m_current_time >= m_pulse_time)) { //SendPulse
    m_pulse_ready = false; //resets
    //Build instance specific portions of pulse spec
    m_pulse.set_time(m_current_time);
    m_pulse.set_x(m_osx);
    m_pulse.set_y(m_osy);
    postMessage("VIEW_RANGE_PULSE", m_pulse.get_spec());
  }


  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

