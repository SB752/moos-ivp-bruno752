/*****************************************************************/
/*    NAME: L. Gajski,                                          */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: BHV_Scout.h                                          */
/*    DATE: April 30th 2022                                      */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/
 
#ifndef BHV_SCOUT_HEADER
#define BHV_SCOUT_HEADER

#include <string>
#include "IvPBehavior.h"
#include "XYPoint.h"
#include "XYPolygon.h"

class BHV_Scout : public IvPBehavior {
public:
  BHV_Scout(IvPDomain);
  ~BHV_Scout() {};
  
  bool         setParam(std::string, std::string);
  void         onIdleState();
  IvPFunction* onRunState();
  void         onEveryState(std::string);
  
protected:
  IvPFunction* buildFunction();
  void         updateScoutPath();
  void         rotateLoiter(double angle);
  void         recenterLoiter();
  void         postViewPoint(bool viewable=true);
  void         postViewablePath();
  void         vizRecenterMode();

protected: // State variables
  double   m_osx;
  double   m_osy;
  double   m_curr_time;

  double   m_ptx;
  double   m_pty;
  bool     m_pt_set;

  XYPolygon m_rescue_region;

  double   m_rescue_height;
  double   m_rescue_width;

  std::vector<XYPoint> m_waypoints;  // Store search pattern waypoints
  std::vector<XYPoint> m_recenter_points; // Store recenter points
  unsigned int m_current_waypoint;   // Track current waypoint index

  int m_cycle_count;

  double m_total_rotation;

  int m_recenter_index;    // Track current recenter point

  double m_last_recenter_time;

protected: // Config variables
  double m_capture_radius;
  double m_desired_speed;
  std::string m_recenter_mode;  // "oval" , "rectangle" , "x"
  std::string m_loiter_mode;    // "vs" - vector sierra , "hg" - hourglass

  std::string m_tmate;
};

#define IVP_EXPORT_FUNCTION
extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Scout(domain);}
}
#endif
