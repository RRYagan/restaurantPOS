# --- Core Entities Module ---
add_library(core_entities
    core/entities/menuitem.h
    core/entities/order.h
    core/entities/table.h
    core/entities/money.h
)
target_include_directories(core_entities PUBLIC core/entities)
target_link_libraries(core_entities PUBLIC Qt6::Core)

# --- Database Module ---
# This remains the single source of truth for database logic
add_library(database
    databasemanager.cpp
    databasemanager.h
)
target_include_directories(database PUBLIC .) # Expose current dir for headers
target_link_libraries(database
    PUBLIC
        Qt6::Sql
        core_entities
    PRIVATE
        Qt6::Core
)

# --- Order Module ---
add_library(order
    orderservice.cpp
    orderservice.h
)
target_include_directories(order PUBLIC .)
target_link_libraries(order
    PUBLIC
        core_entities
    PRIVATE
        Qt6::Core
        Qt6::Sql
)


# --- Menu Module ---
add_library(menu
    menumodel.cpp
    menumodel.h
)
target_include_directories(menu PUBLIC .)
target_link_libraries(menu
    PUBLIC
        core_entities
    PRIVATE
        Qt6::Core
        Qt6::Sql
)
