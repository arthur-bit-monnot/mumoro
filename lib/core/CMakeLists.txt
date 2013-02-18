SET(core_SOURCES 
    path_algo.cpp 
    graph_wrapper.cpp
    reglc_graph.cpp
   )
   
set(CMAKE_CXX_FLAGS -Wall)

add_executable(path-exe ${core_SOURCES})
target_link_libraries(path-exe boost_serialization)

# FIND_PACKAGE(Boost)
# IF (Boost_FOUND)
#     INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
#     ADD_DEFINITIONS( "-DHAS_BOOST" )
# ENDIF()
# 
# FIND_PACKAGE(SWIG REQUIRED)
# SET (UseSWIG ON)
# 
# INCLUDE(${SWIG_USE_FILE})
# 
# FIND_PACKAGE(PythonLibs)
# INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
# 
# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
# 
# SET(CMAKE_SWIG_FLAGS "")
# 
# SET_SOURCE_FILES_PROPERTIES(mumoro.i PROPERTIES CPLUSPLUS ON)
# SET_SOURCE_FILES_PROPERTIES(mumoro.i PROPERTIES SWIG_FLAGS "-includeall")
# 
# SWIG_ADD_MODULE(mumoro python mumoro.i graph_wrapper.cpp)
# SWIG_LINK_LIBRARIES(mumoro ${PYTHON_LIBRARIES})