/************************************************************/
/*    NAME: Stephen Bruno                                   */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                    */
/*    DATE: March 6th, 2025                                  */
/************************************************************/

#ifndef PointReader_HEADER
#define PointReader_HEADER

#include "MBUtils.h"
#include <string>

class PointReader
{
public:
  PointReader();
  ~PointReader();
  void intake(std::string s);

  double get_x();
  double get_y();
  int get_id();

  void set_x(double x);
  void set_y(double y);
  void set_id(int id);

  std::string get_string();


private:
  double m_x_val;
  double m_y_val;
  int m_id;

};

#endif