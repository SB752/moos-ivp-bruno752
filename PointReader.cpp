/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                    */
/*    DATE: March 6th, 2025                                  */
/************************************************************/

#include "PointReader.h"


//---------------------------------------------------------
// Constructor()
PointReader::PointReader() {
    m_x_val = 0;
    m_y_val = 0;
    m_id = 0;
}

//---------------------------------------------------------
// Destructor
PointReader::~PointReader() {
}

//---------------------------------------------------------
//Read string to x,y, and id
//"x=-25,y=-25,id=1"

void PointReader::intake(std::string s){
    std::string temp = biteStringX(s, '=');
    m_x_val = std::stod(BiteStringX(s, ','));
    temp = biteStringX(s, '=');
    m_y_val = std::stod(BiteStringX(s, ','));
    temp = biteStringX(s, '=');
    m_id = stoi(s);
}

