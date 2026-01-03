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



