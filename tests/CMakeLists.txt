add_executable(test_database
    test_database.cpp
)
target_link_libraries(test_database PRIVATE qp_data)

add_test(NAME db COMMAND test_database)
