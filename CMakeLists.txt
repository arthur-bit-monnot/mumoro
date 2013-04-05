cmake_minimum_required (VERSION 2.6)
project (MUMORO) 

OPTION(VERBOSE "Debug output with CWD" OFF)
OPTION(TESTS   "Build tests"           OFF)



add_subdirectory ( algorithms )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g" )

INCLUDE_DIRECTORIES( lib/core )
INCLUDE_DIRECTORIES( ${ALGO_INC_DIRS} )


SET(CORE_SOURCES
    lib/core/graph_wrapper.cpp
    lib/core/duration.cpp
    lib/core/path_algo.cpp
    lib/core/nodes_filter.cpp
    lib/core/utils.cpp
    )
    
set( SOURCES ${SOURCES} ${CORE_SOURCES} )
    
SET(MAIN_EXE_SOURCES
    ${MAIN_EXE_SOURCES}
    ${SOURCES}
    )
    
add_executable( main-exe ${MAIN_EXE_SOURCES} )
target_link_libraries( main-exe boost_serialization ) 
IF(VERBOSE)
  target_link_libraries( main-exe debug cwd )
ENDIF(VERBOSE)

IF(TESTS)
    set( TEST_SOURCES 
         ${TESTS_MAIN}
         ${SOURCES}
       )
    add_executable( tests ${TEST_SOURCES} )
    target_link_libraries( tests boost_serialization ) 
#     set_target_properties( tests PROPERTIES EXCLUDE_FROM_ALL ON)
ENDIF(TESTS)




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

add_custom_command(TARGET swig POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_BINARY_DIR}/_mumoro.so ${CMAKE_SOURCE_DIR}/lib/core/_mumoro.so)
                       
add_custom_command(TARGET swig POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_BINARY_DIR}/mumoro.py ${CMAKE_SOURCE_DIR}/lib/core/mumoro.py)


