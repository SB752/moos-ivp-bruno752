#--------------------------------------------------------
# The CMakeLists.txt for:                       pGenRescue
# Author(s):                              Stephen Bruno
#--------------------------------------------------------

SET(SRC
  GenRescue.cpp
  GenRescue_Info.cpp
  main.cpp
  ../pPointAssign/PointReader.cpp
)

ADD_EXECUTABLE(pGenRescue ${SRC})

TARGET_LINK_LIBRARIES(pGenRescue
   ${MOOS_LIBRARIES}
   contacts
   geometry
   ufield
   apputil
   mbutil
   m
   pthread)

