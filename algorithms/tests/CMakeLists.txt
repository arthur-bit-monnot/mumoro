include_directories(../MultipleParticipants/)
include_directories(../utils/)
include_directories(../RegLC/)
include_directories(../DataStructures/)

set( TESTS ON PARENT_SCOPE )
set( TESTS_MAINS 
     ${CMAKE_CURRENT_SOURCE_DIR}/TestCarPooling.cpp 
     PARENT_SCOPE )
