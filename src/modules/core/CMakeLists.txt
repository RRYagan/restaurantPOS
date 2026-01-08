# --- Core Entities Module ---
add_library(PosCoreEntities
    menuitem.h
    order.h
    orderitem.h
    table.h
    money.h
    usersession.h
    inventoryitem.h
    user.h

)
target_include_directories(PosCoreEntities PUBLIC .)
target_link_libraries(PosCoreEntities PUBLIC Qt6::Core)

# --- Database Module ---
# This remains the single source of truth for database logic
add_library(PosCoreDatabase
    databasemanager.h databasemanager.cpp
    universalfilterproxy.h universalfilterproxy.cpp
    basemodel.h basemodel.cpp
)
target_include_directories(PosCoreDatabase PUBLIC .) # Expose current dir for headers
target_link_libraries(PosCoreDatabase
    PUBLIC
        Qt6::Sql
        PosCoreEntities

    PRIVATE
        Qt6::Core
)





