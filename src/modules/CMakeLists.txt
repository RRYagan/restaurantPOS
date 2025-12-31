# --- Core Entities Module ---
# I moved this up so other modules can reference it
add_library(core_entities
    core/entities/menuitem.h
    core/entities/order.h
    core/entities/table.h
    core/entities/money.h
)
target_include_directories(core_entities PUBLIC core/entities)
target_link_libraries(core_entities
    PUBLIC   # Changed to PUBLIC because .h files need Qt headers
        Qt6::Core
)

# --- Database Module ---
add_library(database
    database/databasemanager.cpp
    database/databasemanager.h
)
target_include_directories(database PUBLIC database)
target_link_libraries(database
    PUBLIC
        Qt6::Sql
        core_entities
    PRIVATE
        Qt6::Core
)

# --- Order Module ---
add_library(order
    order/orderservice.cpp
    order/orderservice.h
)
target_include_directories(order PUBLIC order)
target_link_libraries(order
    PUBLIC
        core_entities
    PRIVATE
        Qt6::Core
        Qt6::Sql
)


# --- Order Module ---
add_library(menu
    menu/menumodel.cpp
    menu/menumodel.h
)
target_include_directories(menu PUBLIC menu)
target_link_libraries(menu
    PUBLIC
        core_entities
    PRIVATE
        Qt6::Core
        Qt6::Sql
)
