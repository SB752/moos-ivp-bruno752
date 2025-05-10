#--------------------------------------------------------
# The CMakeLists.txt for:                   lib_bhv_scout
# Author(s):                                   M.Benjamin
#--------------------------------------------------------
#                                               BHV_Scout
#--------------------------------------------------------
ADD_LIBRARY(BHV_Scout SHARED 
   BHV_Scout.cpp)
TARGET_LINK_LIBRARIES(BHV_Scout
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
)


