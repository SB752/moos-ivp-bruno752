/*****************************************************************/
/*    NAME: L. Gajski                                         */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: BHV_Scout.cpp                                        */
/*    DATE: April 30th 2022                                      */
/*****************************************************************/

#include <cstdlib>
#include <math.h>
#include "BHV_Scout.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "BuildUtils.h"
#include "GeomUtils.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include "XYFormatUtilsPoly.h"

using namespace std;

//-----------------------------------------------------------
// Constructor()

BHV_Scout::BHV_Scout(IvPDomain gdomain) : 
  IvPBehavior(gdomain)
{
  IvPBehavior::setParam("name", "scout");
 
  // Default values for behavior state variables
  m_osx  = 0;
  m_osy  = 0;

  // All distances are in meters, all speed in meters per second
  // Default values for configuration parameters 
  m_desired_speed  = 1; 
  m_capture_radius = 10;
  m_recenter_mode == "oval";

  m_pt_set = false;

  m_rescue_height = 0;
  m_rescue_width  = 0;

  m_current_waypoint = 0;
  m_cycle_count = 0;
  m_recenter_index = 0;

  m_total_rotation = 0;
  m_rotation_angle = 0;

  //m_curr_time = 0;
  //m_last_rotate_time = MOOSTime();    // Initialize with current time[2]
  //m_last_recenter_time = MOOSTime();  // Prevent NaN in time calculations[4]
  
  addInfoVars("NAV_X, NAV_Y");
  addInfoVars("RESCUE_REGION");
  addInfoVars("SCOUTED_SWIMMER");
}

//---------------------------------------------------------------
// Procedure: setParam() - handle behavior configuration parameters

bool BHV_Scout::setParam(string param, string val) 
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);
  
  bool handled = true;
  if(param == "capture_radius")
    handled = setPosDoubleOnString(m_capture_radius, val);
  else if(param == "desired_speed")
    handled = setPosDoubleOnString(m_desired_speed, val);
  else if(param == "tmate")
    handled = setNonWhiteVarOnString(m_tmate, val);
  else if(param == "recenter_mode") {
    if(tolower(val) == "rectangle")
      m_recenter_mode = "rectangle";
    else 
      m_recenter_mode = "oval";
    handled = true;
  } 
  else
    handled = false;

  srand(time(NULL));
  
  return(handled);
}

//-----------------------------------------------------------
// Procedure: onEveryState()

void BHV_Scout::onEveryState(string str) 
{
  if(!getBufferVarUpdated("SCOUTED_SWIMMER"))
    return;

  string report = getBufferStringVal("SCOUTED_SWIMMER");
  if(report == "")
    return;

  if(m_tmate == "") {
    postWMessage("Mandatory Teammate name is null");
    return;
  }
  postOffboardMessage(m_tmate, "SWIMMER_ALERT", report);
}

//-----------------------------------------------------------
// Procedure: onIdleState()

void BHV_Scout::onIdleState() 
{
  m_curr_time = getBufferCurrTime();
}

//-----------------------------------------------------------
// Procedure: onRunState()

IvPFunction* BHV_Scout::onRunState() 
{
    //m_curr_time = MOOSTime();

    // Get vehicle position
    bool ok1, ok2;
    m_osx = getBufferDoubleVal("NAV_X", ok1);
    m_osy = getBufferDoubleVal("NAV_Y", ok2);
    if(!ok1 || !ok2) {
        postWMessage("No ownship X/Y info in info_buffer.");
        return(0);
    }

  // Initialize waypoints if needed
  if (!m_pt_set) {
      updateScoutPath();
  }
/*
      // Handle 150-second rotation timer
      if((m_curr_time - m_last_rotate_time) >= 150) {
        rotateLoiter(m_total_rotation + 30.0);  // Continuous rotation[4]
        m_last_rotate_time = m_curr_time;
        postViewablePath();
        m_total_rotation += 30.0; // Update total rotation[4]
    }

    // Handle 300-second recenter timer
    if((m_curr_time - m_last_recenter_time) >= 270) {
        recenterLoiter();
        //rotateLoiter(-m_rotation_angle); // Reset rotation for new center[1]
        //m_rotation_angle = 0;
        m_last_recenter_time = m_curr_time;
        vizRecenterMode();
    }
*/

    // Check if we've reached current waypoint
    double dist = hypot((m_ptx-m_osx), (m_pty-m_osy));
    if(dist <= m_capture_radius) {
        m_current_waypoint++;
        
        // Pattern completion check
        if(m_current_waypoint >= m_waypoints.size()) {
            m_current_waypoint = 0;


        // No rotation at all to cover more new, areas in less time

          // /*

            recenterLoiter(); // Move to next oval point[7]

          //*/
        }
        
        // Update target waypoint
        m_ptx = m_waypoints[m_current_waypoint].x();
        m_pty = m_waypoints[m_current_waypoint].y();
    }

    // Post visualizations and generate IvP function
    postMessage("SCOUT_STATUS", "moving_to_waypoint=" + intToString(m_current_waypoint));
    postViewPoint(true);
    return buildFunction();
}

//-----------------------------------------------------------
// Procedure: updateScoutPath()

void BHV_Scout::updateScoutPath()
{
  if (m_pt_set)
    return;
  
  string region_str = getBufferStringVal("RESCUE_REGION");
  if (region_str == "") {
    postWMessage("Unknown RESCUE_REGION");
    return;
  }
  
  XYPolygon region = string2Poly(region_str);
  if (!region.is_convex()) {
    postWMessage("Badly formed RESCUE_REGION");
    return;
  }
  
  // Calculate the height, width, and center of the rescue region
  double min_x = region.get_min_x();
  double max_x = region.get_max_x();
  double min_y = region.get_min_y();
  double max_y = region.get_max_y();

  double area = region.area();
  double perim = region.perim();

  m_rescue_height = (perim + sqrt(perim*perim - 16*area))/4;
  m_rescue_width = area / m_rescue_height;

  double center_x = (min_x + max_x) / 2.0;
  double center_y = (min_y + max_y) / 2.0;
  double theta = 26 * M_PI / 180.0; // Angle for triangle points ... 26 deg

  if(m_recenter_mode == "rectangle") {
    // Generate rectangle vertices rotated 26° with 3/4 dimensions
    double rect_length = 0.65 * m_rescue_height;
    double rect_width = 0.40 * m_rescue_width;
    double theta = 26 * M_PI / 180.0;
    
    // Define 6 points: 4 corners + top/bottom midpoints
    vector<XYPoint> corners = {
      {0,              rect_width/2},  // Top midpoint (now index 0)
      {-rect_length/2, rect_width/2},  // NW corner (index 1) 
      {-rect_length/2, -rect_width/2}, // SW corner (index 2) 
      {0,              -rect_width/2}, // Bottom midpoint (index 3)                
      {rect_length/2,  -rect_width/2},  // SE corner (index 4)      
      {rect_length/2,  rect_width/2}  // NE corner (index 5)
    };

    m_recenter_points.clear();
    for(int i = 0; i <= 5; i++) {
        // Apply rotation and translation to rescue center
        double x = center_x + (corners[i].x()*cos(theta) - corners[i].y()*sin(theta));
        double y = center_y + (corners[i].x()*sin(theta) + corners[i].y()*cos(theta));
        m_recenter_points.push_back(XYPoint(x,y));
    }
    vizRecenterMode();
  }
  else {
    // Calculate oval dimensions
    double oval_length = 0.75 * m_rescue_height;
    double oval_height = 0.50 * m_rescue_width;

    // Generate 8 equally spaced recenter points on the oval
    m_recenter_points.clear();
    for (int i = 0; i < 8; i++) {
      double angle = (i + 2) * (2 * M_PI / 8); // Divide the oval into 8 segments
      double x = center_x + (oval_length / 2) * cos(angle) * cos(theta) -
                  (oval_height / 2) * sin(angle) * sin(theta);
      double y = center_y + (oval_length / 2) * cos(angle) * sin(theta) +
                  (oval_height / 2) * sin(angle) * cos(theta);
      m_recenter_points.push_back(XYPoint(x, y));
    }
    vizRecenterMode(); 
  }

  // Shift the starting center point to the top midpoint of the oval
  m_waypoints.clear();
  m_waypoints.push_back(m_recenter_points[0]); // Top midpoint of the oval
  
  // Calculate triangle dimensions - each will be 1/4 of region dimensions
  double vs_center_x = m_recenter_points[0].x();
  double vs_center_y = m_recenter_points[0].y();
  double triangle_height = m_rescue_height / 8.0;
  double triangle_width = m_rescue_width / 8.0;
  
  // Store information about the rescue region
  postMessage("RESCUE_INFO", "width=" + doubleToStringX(m_rescue_width) + 
              ",height=" + doubleToStringX(m_rescue_height) + 
              ",center_x=" + doubleToStringX(center_x) + 
              ",center_y=" + doubleToStringX(center_y));
  
  // Create waypoints vector
  m_waypoints.clear();
  XYPoint vs_center(vs_center_x, vs_center_y);
  
  // Calculate the three triangle points (all pointing downward)
  // Center point is the bottom of all triangles
  
  // Top triangle points
  double tlx = vs_center_x - triangle_width * cos(theta) + triangle_height * sin(theta);
  double tly = vs_center_y - triangle_width * sin(theta) - triangle_height * cos(theta);
  
  double trx = vs_center_x + triangle_width * cos(theta) + triangle_height * sin(theta);
  double try_ = vs_center_y + triangle_width * sin(theta) - triangle_height * cos(theta);
  
  // Middle triangle points (left and right)
  double mlx = vs_center_x - 2 * triangle_width * cos(theta);
  double mly = vs_center_y - 2 * triangle_width * sin(theta);
  
  double mrx = vs_center_x + 2 * triangle_width * cos(theta);
  double mry = vs_center_y + 2 * triangle_width * sin(theta);
  
  // Bottom triangle points
  double blx = vs_center_x - triangle_width * cos(theta) - triangle_height * sin(theta);
  double bly = vs_center_y - triangle_width * sin(theta) + triangle_height * cos(theta);
  
  double brx = vs_center_x + triangle_width * cos(theta) - triangle_height * sin(theta);
  double bry = vs_center_y + triangle_width * sin(theta) + triangle_height * cos(theta);
  
  // Create point objects for all vertices
  XYPoint top_left(tlx, tly);
  XYPoint top_right(trx, try_);
  XYPoint middle_left(mlx, mly);
  XYPoint middle_right(mrx, mry);
  XYPoint bottom_left(blx, bly);
  XYPoint bottom_right(brx, bry);
  
  // Define the Victor Sierra search pattern, always moving clockwise
  // First triangle: Center → Top Left → Top Right → Center
  m_waypoints.push_back(vs_center);       // Start at center
  m_waypoints.push_back(bottom_left);     // Move to top left
  m_waypoints.push_back(bottom_right);    // Move to top right
  m_waypoints.push_back(vs_center);       // Back to center
  
  // Second triangle: Center → Bottom Right → Bottom Left → Center
  m_waypoints.push_back(top_left); // Move to bottom left
  m_waypoints.push_back(top_right);  // Move to middle left
  m_waypoints.push_back(vs_center);       // Back to center
  
  /*
  // Third triangle: Center → Middle Left → Middle Right → Center
  m_waypoints.push_back(middle_left);  // Move to middle right
  m_waypoints.push_back(top_left); // Move to bottom right
  m_waypoints.push_back(vs_center);       // Back to center
  */

  // Initialize waypoint tracking
  m_current_waypoint = 0;
  m_ptx = m_waypoints[0].x();
  m_pty = m_waypoints[0].y();
  
  // Create a viewable path with track lines connecting all waypoints
  XYSegList seglist;
  for (unsigned int i=0; i < m_waypoints.size(); i++) {
    seglist.add_vertex(m_waypoints[i].x(), m_waypoints[i].y());
    
    // Also post individual points for easier visualization
    XYPoint point = m_waypoints[i];
    string label = "VS_pt" + intToString(i);
    
    point.set_label(label);
    if (i == 0 || i == 3 || i == 6) {
      // Center points
      point.set_color("vertex", "red");
      point.set_vertex_size(5);
    } else {
      // Other waypoints
      point.set_color("vertex", "yellow");
      point.set_vertex_size(3);
    }
    
    postMessage("VIEW_POINT", point.get_spec());
    postMessage("SCOUT_WAYPOINT", "id=" + intToString(i) + 
                ",x=" + doubleToStringX(point.x()) + 
                ",y=" + doubleToStringX(point.y()));
  }
  
  // Set visual properties for the seglist
  seglist.set_color("vertex", "orange");
  seglist.set_color("edge", "white");
  seglist.set_edge_size(1);
  seglist.set_vertex_size(3);
  seglist.set_label(m_us_name + "_vs_pattern");
  
  // Post the seglist to visualize connections between waypoints
  postMessage("VIEW_SEGLIST", seglist.get_spec());

  m_pt_set = true;
}

//-----------------------------------------------------------
// Procedure: rotateLoiter()

void BHV_Scout::rotateLoiter(double angle) 
{
    double rotation_angle = angle * M_PI / 180.0; // Convert angle to radians

    // Get the center of the pattern (assume the first waypoint is the center)
    if (m_waypoints.empty()) {
        postWMessage("Waypoints are empty, cannot rotate loiter pattern.");
        return;
    }

    double center_x = m_waypoints[0].x();
    double center_y = m_waypoints[0].y();

    // Rotate each waypoint around the center
    for (auto &point : m_waypoints) {
        double x = point.x() - center_x;
        double y = point.y() - center_y;

        double rotated_x = x * cos(rotation_angle) - y * sin(rotation_angle);
        double rotated_y = x * sin(rotation_angle) + y * cos(rotation_angle);

        point.set_vertex(rotated_x + center_x, rotated_y + center_y);
    }

    // Update the visualization of the rotated pattern
    postViewablePath();
}



void BHV_Scout::recenterLoiter() 
{
  static int recenter_index = 0; // Track the current recenter point

  if (m_recenter_mode == "rectangle") {
    // Check if the recenter points are set correctly
    if (m_recenter_points.size() != 6) {
      postWMessage("Missing recenter points");
      return;
    }
  } else if (m_recenter_mode == "oval") {
    // Check if the recenter points are set correctly
    if (m_recenter_points.size() != 8) {
      postWMessage("Missing recenter points");
      return;
    }
  }

  // Update the center point to the next recenter point on the oval
  m_recenter_index = (m_recenter_index + 1) % m_recenter_points.size();
  XYPoint new_center = m_recenter_points[m_recenter_index];

  // Clear the waypoints and recalculate the triangular loiter pattern
  m_waypoints.clear();

  double vs_center_x = new_center.x();
  double vs_center_y = new_center.y();
  double theta = 26 * M_PI / 180.0; // Angle for triangle alignment (26 degrees)

  // Dimensions for the triangles
  double triangle_height = m_rescue_height / 8.0;
  double triangle_width = m_rescue_width / 8.0;

  // Top triangle points
  double tlx = vs_center_x - triangle_width * cos(theta) + triangle_height * sin(theta);
  double tly = vs_center_y - triangle_width * sin(theta) - triangle_height * cos(theta);
  
  double trx = vs_center_x + triangle_width * cos(theta) + triangle_height * sin(theta);
  double try_ = vs_center_y + triangle_width * sin(theta) - triangle_height * cos(theta);
  
  // Middle triangle points (left and right)
  double mlx = vs_center_x - 2 * triangle_width * cos(theta);
  double mly = vs_center_y - 2 * triangle_width * sin(theta);
  
  double mrx = vs_center_x + 2 * triangle_width * cos(theta);
  double mry = vs_center_y + 2 * triangle_width * sin(theta);
  
  // Bottom triangle points
  double blx = vs_center_x - triangle_width * cos(theta) - triangle_height * sin(theta);
  double bly = vs_center_y - triangle_width * sin(theta) + triangle_height * cos(theta);
  
  double brx = vs_center_x + triangle_width * cos(theta) - triangle_height * sin(theta);
  double bry = vs_center_y + triangle_width * sin(theta) + triangle_height * cos(theta);
  
  // Create point objects for all vertices
  XYPoint vs_center(vs_center_x, vs_center_y);
  XYPoint top_left(tlx, tly);
  XYPoint top_right(trx, try_);
  XYPoint middle_left(mlx, mly);
  XYPoint middle_right(mrx, mry);
  XYPoint bottom_left(blx, bly);
  XYPoint bottom_right(brx, bry);
  
  // Define the Victor Sierra search pattern, always moving clockwise
  // First triangle: Center → Top Left → Top Right → Center
  m_waypoints.push_back(vs_center);       // Start at center
  m_waypoints.push_back(bottom_left);     // Move to top left
  m_waypoints.push_back(bottom_right);    // Move to top right
  m_waypoints.push_back(vs_center);       // Back to center
  
  // Second triangle: Center → Bottom Right → Bottom Left → Center
  m_waypoints.push_back(top_left); // Move to bottom left
  m_waypoints.push_back(top_right);  // Move to middle left
  m_waypoints.push_back(vs_center);       // Back to center
  
  // Reset waypoint tracking
  m_current_waypoint = 0;
  m_ptx = m_waypoints[0].x();
  m_pty = m_waypoints[0].y();

  // Update the visualization
  postViewablePath();
}

//-----------------------------------------------------------
// Procedure: postViewPoint()

void BHV_Scout::postViewPoint(bool viewable) 
{

  XYPoint pt(m_ptx, m_pty);
  pt.set_vertex_size(5);
  pt.set_vertex_color("orange");
  pt.set_label(m_us_name + "'s next waypoint");
  
  string point_spec;
  if(viewable)
    point_spec = pt.get_spec("active=true");
  else
    point_spec = pt.get_spec("active=false");
  postMessage("VIEW_POINT", point_spec);
}

//-----------------------------------------------------------
// Procedure: postViewablePath()

void BHV_Scout::postViewablePath()
{
  if (m_waypoints.size() < 2)
    return;
    
  XYSegList seglist;
  
  // Add all waypoints to the seglist in order
  for (unsigned int i=0; i < m_waypoints.size(); i++) {
    seglist.add_vertex(m_waypoints[i].x(), m_waypoints[i].y());
  }
  
  // Set visual properties
  seglist.set_color("vertex", "orange");
  seglist.set_color("edge", "white");
  seglist.set_edge_size(1);
  seglist.set_vertex_size(3);
  seglist.set_label(m_us_name + "_vs_pattern");
  
  // Post the seglist to visualize connections between waypoints
  postMessage("VIEW_SEGLIST", seglist.get_spec());
}

//-----------------------------------------------------------
// Procedure: vizRecenterMode()

void BHV_Scout::vizRecenterMode() {
  if(m_recenter_points.empty())
  return;

  XYSegList shape;
  for(auto& pt : m_recenter_points)
    shape.add_vertex(pt.x(), pt.y());
    
  // Style based on mode
  if(m_recenter_mode == "rectangle") {
    shape.add_vertex(m_recenter_points[0].x(), m_recenter_points[0].y());
    shape.set_color("edge", "dodger_blue");
    shape.set_label(m_us_name + "_recenter_rect"); 
  }
  else {
    shape.add_vertex(m_recenter_points[0].x(), m_recenter_points[0].y());
    shape.set_color("edge", "white");
    shape.set_label(m_us_name + "_recenter_oval");
  }
  
  postMessage("VIEW_SEGLIST", shape.get_spec());
}


//-----------------------------------------------------------
// Procedure: buildFunction()

IvPFunction *BHV_Scout::buildFunction() 
{
  if(!m_pt_set)
    return(0);
  
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(m_desired_speed);
  spd_zaic.setPeakWidth(0.5);
  spd_zaic.setBaseWidth(1.0);
  spd_zaic.setSummitDelta(0.8);  
  if(spd_zaic.stateOK() == false) {
    string warnings = "Speed ZAIC problems " + spd_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }
  
  double rel_ang_to_wpt = relAng(m_osx, m_osy, m_ptx, m_pty);
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(rel_ang_to_wpt);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }

  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ivp_function = coupler.couple(crs_ipf, spd_ipf, 50, 50);

  return(ivp_function);
}
