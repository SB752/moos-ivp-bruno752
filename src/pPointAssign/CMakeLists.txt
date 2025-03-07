#--------------------------------------------------------
# The CMakeLists.txt for:                       pPointAssign
# Author(s):                              Stephen Bruno
#--------------------------------------------------------

SET(SRC
  PointAssign.cpp
  PointAssign_Info.cpp
  PointReader.cpp
  main.cpp

)

ADD_EXECUTABLE(pPointAssign ${SRC})

TARGET_LINK_LIBRARIES(pPointAssign
   ${MOOS_LIBRARIES}
   apputil
   geometry
   mbutil
   m
   pthread)

