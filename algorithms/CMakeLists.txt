
add_subdirectory( utils )
add_subdirectory( RegLC )
add_subdirectory( MultipleParticipants )
add_subdirectory( tests )

## This might be used to add module with no change in build configuration. However it makes it hard for IDEs to track which directories 
## are in build configuration
## Add all subdirectories containing CMakeLists.txt
# file(GLOB all_valid_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*/CMakeLists.txt")
# 
# foreach(dir ${all_valid_subdirs})
#     message(STATUS "path = ${dir}")
#     if(${dir} MATCHES "^([^/]*)//CMakeLists.txt")
#         string(REGEX REPLACE "^([^/]*)//CMakeLists.txt" "\\1" dir_trimmed ${dir})
#         add_subdirectory(${dir_trimmed})
#     endif()
# endforeach(dir)


# Share the following variables with parent directory
set( SWIG_SOURCES ${SWIG_SOURCES} PARENT_SCOPE )
set( SOURCES ${SOURCES} PARENT_SCOPE )
set( MAIN_EXE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp PARENT_SCOPE )
set( ALGO_INC_DIRS ${ALGO_INC_DIRS} PARENT_SCOPE )


set( TESTS ${TEST} PARENT_SCOPE )
set( TESTS_MAIN ${TESTS_MAIN} PARENT_SCOPE )
