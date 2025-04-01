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
#include "ZAIC_PEAK.h"
#include "BHV_ZigLeg.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_ZigLeg::BHV_ZigLeg(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y","NAV_HEADING");
  addInfoVars("WPT_INDEX","no_warning");



}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_ZigLeg::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "zig_angle") && isNumber(val)) {
    // Set local member variables here
    m_zig_angle = double_val;
    return(true);
  }
  else if((param=="zig_duration") && isNumber(val)) {
    m_zig_duration = double_val;
    return(true);
  } 
  else if((param=="zig_delay") && isNumber(val)) {
    m_zig_delay = double_val;
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

void BHV_ZigLeg::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_ZigLeg::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_ZigLeg::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_ZigLeg::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_ZigLeg::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_ZigLeg::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_ZigLeg::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_ZigLeg::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;

  m_current_index = getBufferDoubleVal("WPT_INDEX");
  m_current_time = getBufferCurrTime();
  m_osx = getBufferDoubleVal("NAV_X");
  m_osy = getBufferDoubleVal("NAV_Y");

  if (m_current_index != m_previous_index) { //Get Pulse Ready
    m_zig_ready = true;
    m_zig_start_time = m_current_time + m_zig_delay;
    m_previous_index = m_current_index;

  }

  //Get base course for objective function
  if((m_current_time > (m_zig_start_time-1)) && (m_current_time <(m_zig_start_time))) {
    m_nav_heading = getBufferDoubleVal("NAV_HEADING");
  }

  if(m_zig_ready && (m_current_time > m_zig_start_time)) { //Produce Zig
    ipf = buildFunctionWithZAIC();
    if (m_current_time > m_zig_start_time + m_zig_duration) { //Reset
      m_zig_ready = false;
    }

  }
  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

//-----------------------------------------------------------
// Procedure: buildFunctionWithZAIC

IvPFunction *BHV_ZigLeg::buildFunctionWithZAIC() 
{
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit((m_nav_heading + m_zig_angle)); //Nav Heading fixed based on 1 sec before start
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();

  return(crs_ipf);
}
