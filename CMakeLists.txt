cmake_minimum_required (VERSION 2.6)
project (MUMORO) 

OPTION(VERBOSE "Debug output with CWD" OFF)
OPTION(TESTS   "Build tests"           OFF)

IF(TESTS)
    ENABLE_TESTING()
ENDIF(TESTS)


add_subdirectory ( lib )
add_subdirectory ( algorithms )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g" )

INCLUDE_DIRECTORIES( lib/core )
INCLUDE_DIRECTORIES( ${ALGO_INC_DIRS} )


SET(CORE_SOURCES
    lib/core/graph_wrapper.cpp
    lib/core/duration.cpp
    lib/core/path_algo.cpp
    lib/core/nodes_filter.cpp
    )
    
SET(MAIN_EXE_SOURCES
    ${MAIN_EXE_SOURCES}
    ${CORE_SOURCES}
    )

add_executable( main-exe ${MAIN_EXE_SOURCES} )
target_link_libraries( main-exe boost_serialization ) 
IF(VERBOSE)
  target_link_libraries( main-exe debug cwd )
ENDIF(VERBOSE)



FIND_PACKAGE(SWIG REQUIRED)
SET (UseSWIG ON)

INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

SET(SWIG_SOURCES
    ${SWIG_SOURCES}
    ${CORE_SOURCES}
    )


SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(interface.i PROPERTIES CPLUSPLUS ON)

SWIG_ADD_MODULE(mumoro python interface.i ${SWIG_SOURCES})
SWIG_LINK_LIBRARIES(mumoro ${PYTHON_LIBRARIES} boost_serialization)


set_target_properties(_mumoro PROPERTIES EXCLUDE_FROM_ALL ON)
add_custom_target( swig )
add_dependencies( swig _mumoro )


