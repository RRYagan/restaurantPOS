cmake_minimum_required(VERSION 3.16)
project(RestaurantPOSTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find only the components needed for testing
find_package(Qt6 REQUIRED COMPONENTS Core Sql Test)

# 2. Define the test executable
# We include the source files from the parent directory so we can test them
qt_add_executable(RestaurantPOSTest
    tst_restaurantpos.cpp

)

# 3. Add Include Paths
# This allows tst_restaurantpos.cpp to find your headers (e.g., #include "money.h")
target_include_directories(RestaurantPOSTest PRIVATE ../src)

# 4. Link Libraries
# Qt6::Test is the key requirement here
target_link_libraries(RestaurantPOSTest PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Test
    database
    POS_Sales
    menu
)

# 5. Enable CTest
enable_testing()
add_test(NAME RestaurantPOSTest COMMAND RestaurantPOSTest)
