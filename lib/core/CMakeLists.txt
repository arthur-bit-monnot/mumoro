SET(core_SOURCES 
    path_algo.cpp 
    graph_wrapper.cpp
    reglc_graph.cpp
    duration.cpp
    debug/cwd_debug.cc
    debug/stack_trace.cpp
   )
   
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

IF(VERBOSE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCWDEBUG")
ENDIF(VERBOSE)


add_executable(path-exe ${core_SOURCES} main.cpp)
target_link_libraries( path-exe boost_serialization)
IF(VERBOSE)
  target_link_libraries( path-exe debug cwd)
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



