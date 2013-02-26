


include_directories(${INCLUDE_DIRECTORIES} ..)

add_executable(tester reg_lc_test.cpp )

target_link_libraries( tester 
                       core
                       boost_unit_test_framework)

add_executable( perf performance.cpp )

target_link_libraries( perf core )

                        
ADD_TEST(Dijkstra_on_RegLC_Graph tester)

