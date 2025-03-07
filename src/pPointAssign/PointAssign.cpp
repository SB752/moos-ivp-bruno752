/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.cpp                                 */
/*    DATE: March 4th, 2025                                 */
/*    UPDATE: March 7th, 2025                               */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "PointAssign.h"
#include "PointReader.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

PointAssign::PointAssign()
{
  m_ship_names;
  m_visit_points;
  m_ship_points_A;
  m_ship_points_B;

  m_assignment_method = "default"; //Odds and evens or East/west

  m_ship_reg_time = 0;

  m_start_flag = "firstpoint";
  m_end_flag = "lastpoint";

  m_first_reading = false;
  m_last_reading = false;

  m_sort_complete = false;
  m_point_pub_complete = false;

}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

    /*if(key == "NODE_REPORT") {
      //Collects unique ship names for distribution
      if(find(m_ship_names.begin(), m_ship_names.end(), msg.GetCommunity()) != m_ship_names.end()){
        m_ship_names.push_back(msg.GetCommunity());
        m_ship_reg_time = msg.GetTime();
     }
    }*/ //^^Draft Code for autodetecting ships

    if(key == "VISIT_POINT") { //Collects visit points, stops collecting once end reached
      if (msg.GetString() == m_start_flag) {
        m_first_reading = true;
      } else if (msg.GetString() == m_end_flag){
        m_last_reading = true;
      } else {  //Transmits coordinates to vector of PointReader classes
        PointReader mestemp;
        mestemp.intake(msg.GetString());
        m_visit_points.push_back(mestemp);
      }
      //Notify("POINT_UNREAD","false");  <-if necessary for handshake
    }

/*
    else if(key == "DB_TIME") {
      m_current_time = msg.GetDouble();
    }
    */
    else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool PointAssign::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
  AppCastingMOOSApp::Iterate();
  
  //Sort Points Based on selected method
  if(m_last_reading && !m_sort_complete){  //Verify all readings recieved before sorting, haven't sorted already
    //Sort points based on method
    //Default: Odds and Evens
    if(m_assignment_method == "default"){
      for(int i = 0; i<m_visit_points.size(); i++){
        if(m_visit_points[i].get_id() % 2 == 0){
          m_ship_points_A.push_back(m_visit_points[i]);
        } else {
          m_ship_points_B.push_back(m_visit_points[i]);
        }
      }
    } else{  //Alternative: East/West
      //Calculate Average X
      double avg_x = 0;
      for(int i = 0; i<m_visit_points.size(); i++){
        avg_x += m_visit_points[i].get_x();
      }
      avg_x = avg_x/m_visit_points.size();
      //Split points based on average X
      for(int i = 0; i<m_visit_points.size(); i++){
        if(m_visit_points[i].get_x() > avg_x){
          m_ship_points_A.push_back(m_visit_points[i]);
        } else {
          m_ship_points_B.push_back(m_visit_points[i]);
        }
      }
    }
    m_sort_complete = true;

  }

  if(m_sort_complete && !m_point_pub_complete){ //Publish points to MOOSDB
    Notify("VISIT_POINT_"+m_ship_names[0], m_start_flag);
    Notify("VISIT_POINT_"+m_ship_names[1], m_start_flag);

    for(int i = 0; i < m_ship_points_A.size(); i++){
      XYPoint point(m_ship_points_A[i].get_x(), m_ship_points_A[i].get_y());
      point.set_label(to_string(m_ship_points_A[i].get_id()));
      point.set_color("vertex","red");
      point.set_param("vertex_size","4");
      Notify("VIEW_POINT", point.get_spec());
      Notify("VISIT_POINT_"+m_ship_names[0], m_ship_points_A[i].get_string());
      //Notify("TEST_A_"+to_string(i), m_ship_points_A[i].get_string());
    }
    for(int i = 0; i < m_ship_points_B.size(); i++){
      XYPoint point(m_ship_points_B[i].get_x(), m_ship_points_B[i].get_y());
      point.set_label(to_string(m_ship_points_B[i].get_id()));
      point.set_color("vertex","blue");
      point.set_param("vertex_size","4");
      Notify("VIEW_POINT", point.get_spec());
      Notify("VISIT_POINT_"+m_ship_names[1], m_ship_points_B[i].get_string());
      //Notify("TEST_B_"+to_string(i), m_ship_points_B[i].get_string());
    }

    //Notify("VISIT_POINT_"+m_ship_names[0], m_end_flag);
    //Notify("VISIT_POINT_"+m_ship_names[1], m_end_flag);

    m_point_pub_complete = true;
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "ship_list") {
      m_ship_names = parseString(value, ',');
      handled = true;
    }
    else if(param == "assignment_method") {
      m_assignment_method = value;
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables()

void PointAssign::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("VISIT_POINT",0);
  //Register("NODE_REPORT",0);
  //Register("DB_TIME",0);

}


//------------------------------------------------------------
// Procedure: buildReport()

bool PointAssign::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "Points Retrieved:                           " << endl;
  for(int i = 0; i < m_visit_points.size(); i++){
    m_msgs <<"["<<i<<"]" << m_visit_points[i].get_string() << endl;
  }
  m_msgs << "total points: "<< m_visit_points.size() << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Ships Detected:                             " << endl;
  for(int i = 0; i < m_ship_names.size(); i++){
    m_msgs <<"["<<i<<"]: " << m_ship_names[i] << endl;
  };
  m_msgs << "============================================" << endl;
  m_msgs << "Assignment Method: " << m_assignment_method << endl;
  m_msgs << "============================================" << endl;
  m_msgs << "Ship Assignments:                           " << endl;
  m_msgs << "Ship A: "<< m_ship_names[0] << endl;
  for(int i = 0; i < m_ship_points_A.size(); i++){
    m_msgs <<"["<<i<<"]" << m_ship_points_A[i].get_string() << endl;
  }
  m_msgs << "Ship B: " << m_ship_names[1] << endl; 
  for(int i = 0; i < m_ship_points_B.size(); i++){
    m_msgs <<"["<<i<<"]" << m_ship_points_B[i].get_string() << endl;
  }

  m_msgs << "============================================" << endl;

/*
  ACTable actab(4);
  actab << "Alpha | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << "one" << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();
*/
  return(true);
}

/*
void PointAssign::postViewPoint(double x, double y, string label, string color){
  XYPoint point(x,y);
  point.set_label(label);
  point.set_color("vertex",color);
  point.set_param("vertex_size","4");
  Notify("VIEW_POINT",point.get_spec());
}
  */



