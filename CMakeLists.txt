cmake_minimum_required(VERSION 3.16)
project(pos VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Sql)

# 2. Modern Qt Setup
qt_standard_project_setup(REQUIRES 6.8)

# 3. Add subdirectories (Definitions)
add_subdirectory(src/modules)
add_subdirectory(src/ui/sales)

# 4. Create the main executable
qt_add_executable(RestaurantPOS
    src/main.cpp
    # Add your main.qml here if you have one in the root
)

# 5. Link EVERYTHING here
# This connects the executable to your C++ logic AND your QML UI modules
target_link_libraries(RestaurantPOS
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        POS_Sales
        POS_Salesplugin
        database
        core_entities
        order
        menu
)

# 6. OS Specific Properties
set_target_properties(RestaurantPOS PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
