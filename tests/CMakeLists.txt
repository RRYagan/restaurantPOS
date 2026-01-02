cmake_minimum_required(VERSION 3.16)
project(RestaurantPOSTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find only the components needed for testing
find_package(Qt6 REQUIRED COMPONENTS Core Sql Test)

# 2. Helper function to reduce boilerplate for new tests
function(add_restaurant_test TEST_NAME SOURCES)
    qt_add_executable(${TEST_NAME} ${SOURCES})

    target_include_directories(${TEST_NAME} PRIVATE ../src)

    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        database    # Linked as requested
        POS_Sales   # Linked as requested
        menu        # Linked as requested
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# 3. Define the test executables
enable_testing()

# Standard Integration Test
add_restaurant_test(RestaurantPOSTest tst_restaurantpos.cpp)

# Standalone SalesModel Test
add_restaurant_test(SalesModelTest tst_salesmodel.cpp)
