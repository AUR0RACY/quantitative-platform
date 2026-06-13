add_executable(test_database
    test_database.cpp
)
target_link_libraries(test_database PRIVATE qp_data qp_engine)

add_executable(test_engine
    test_engine.cpp
)
target_link_libraries(test_engine PRIVATE qp_engine qp_core)

add_test(NAME db     COMMAND test_database)
add_test(NAME engine COMMAND test_engine)
