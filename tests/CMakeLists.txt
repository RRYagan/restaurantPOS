cmake_minimum_required(VERSION 3.16)
project(RestaurantPOSTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# 1. Find only the components needed for testing
find_package(Qt6 REQUIRED COMPONENTS Core Sql Test Quick)


# 2. Helper function to reduce boilerplate for new tests
function(add_restaurant_test TEST_NAME SOURCES)
    qt_add_executable(${TEST_NAME} ${SOURCES})

    target_include_directories(${TEST_NAME} PRIVATE
        ../src/core
        ../src/ui/views
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        Qt6::Quick
        PosCoreDatabase
        POS_UI   
        POS_UIplugin

    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# 3. Define the test executables
enable_testing()

# Standard Integration Test
# add_restaurant_test(RestaurantPOSTest tst_restaurantpos.cpp)

# Standalone SalesView Test
# add_restaurant_test(SalesViewTest tst_saleview.cpp)

# add_restaurant_test(MenuScreenUITest tst_uimenuscreen.cpp)
