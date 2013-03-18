SET(mumoro_SOURCES ${mumoro_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/path_algo.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/graph_wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/duration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nodes_filter.cpp
   )
   
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

IF(VERBOSE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCWDEBUG")
ENDIF(VERBOSE)




# Build a library containing core for tests
IF(TESTS)
    add_library(core ${core_SOURCES})
    target_link_libraries( core boost_serialization )
    
    IF(VERBOSE)
        target_link_libraries( core debug cwd )
    ENDIF(VERBOSE)
    
    add_subdirectory(test)
ENDIF(TESTS)



