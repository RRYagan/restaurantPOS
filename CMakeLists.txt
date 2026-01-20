cmake_minimum_required(VERSION 3.16)
project(pos VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor")

# 1. Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Sql Network)
# find_package(Qt6 REQUIRED COMPONENTS Core)

# 2. Modern Qt Setup
qt_standard_project_setup(REQUIRES 6.8)

# 3. Add subdirectories (Definitions)
add_subdirectory(src/modules/core)
add_subdirectory(src/modules/payment/)

add_subdirectory(src/ui)
add_subdirectory(tests)

# 4. Create the main executable
qt_add_executable(RestaurantPOS
    src/main.cpp
    config.json


    # Add your main.qml here if you have one in the root
)

# 5. Link EVERYTHING here
# This connects the executable to your C++ logic AND your QML UI modules
target_link_libraries(RestaurantPOS
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        Qt6::QuickControls2
        POS_UI
        POS_UIplugin
        PosCoreDatabase
        PosPayment

)
target_link_libraries(RestaurantPOS PRIVATE Qt6::Core)

add_custom_command(TARGET RestaurantPOS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
)

# 6. OS Specific Properties
set_target_properties(RestaurantPOS PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
