/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                    */
/*    DATE: March 6th, 2025                                  */
/************************************************************/

#ifndef PointReader_HEADER
#define PointReader_HEADER

#include "PointAssign.h"

class PointReader : public PointAssign {
public:
  PointReader();
  ~PointReader();
  void intake(std::string s);


private:
  double m_x_val;
  double m_y_val;
  int m_id;

};

#endif