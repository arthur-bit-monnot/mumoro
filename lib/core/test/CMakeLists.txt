

IF(TESTS)
    include_directories(${INCLUDE_DIRECTORIES} ..)

    add_executable(tester reg_lc_test.cpp )
    
    target_link_libraries( tester core )
    target_link_libraries(tester 
                          boost_unit_test_framework)
                          
    ADD_TEST(Dijkstra_on_RegLC_Graph tester)
ENDIF(TESTS)