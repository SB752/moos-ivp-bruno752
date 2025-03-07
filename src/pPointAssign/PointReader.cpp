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
    m_x_val = std::stod(biteStringX(s, ','));
    temp = biteStringX(s, '=');
    m_y_val = std::stod(biteStringX(s, ','));
    temp = biteStringX(s, '=');
    m_id = stoi(s);
}

double PointReader::get_x(){
    return m_x_val;
}

double PointReader::get_y(){
    return m_y_val;
}

int PointReader::get_id(){
    return m_id;
}

void PointReader::set_x(double x){
    m_x_val = x;
}

void PointReader::set_y(double y){
    m_y_val = y;
}

void PointReader::set_id(int id){
    m_id = id;
}

std::string PointReader::get_string(){
    std::string s = "x=" + std::to_string(m_x_val) + ",y=" + std::to_string(m_y_val) + ",id=" + std::to_string(m_id);
    return s;
}


