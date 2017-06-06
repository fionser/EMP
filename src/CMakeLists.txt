add_subdirectory(emp)
add_executable(main main.cpp)
target_link_libraries(main relic gmp ${OPENSSL_LIBRARIES} emp-gc emp-utils)