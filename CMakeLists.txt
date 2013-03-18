cmake_minimum_required (VERSION 2.6)
project (MUMORO) 

OPTION(VERBOSE "Debug output with CWD" OFF)
OPTION(TESTS   "Build tests"           OFF)

IF(TESTS)
    ENABLE_TESTING()
ENDIF(TESTS)


add_subdirectory (lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")



FIND_PACKAGE(SWIG REQUIRED)
SET (UseSWIG ON)

INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(lib/core)

get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message("inc_dirs = ${inc_dirs}")

SET(SWIG_SOURCES
    lib/core/graph_wrapper.cpp
    lib/core/duration.cpp
    lib/core/reglc_graph.cpp
    lib/core/path_algo.cpp
    lib/core/nodes_filter.cpp
    lib/core/muparo.cpp
    )


SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(lib/core/mumoro.i PROPERTIES CPLUSPLUS ON)
# SET_SOURCE_FILES_PROPERTIES(lib/core/mumoro.i PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(mumoro python lib/core/mumoro.i ${SWIG_SOURCES})
SWIG_LINK_LIBRARIES(mumoro ${PYTHON_LIBRARIES} boost_serialization)

