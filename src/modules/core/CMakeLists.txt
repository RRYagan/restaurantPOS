# --- Core Entities Module ---
add_library(PosCoreEntities
    menuitem.h
    order.h
    # orderitem.h
    table.h
    money.h
    usersession.h
    inventoryitem.h
    user.h
    product.h

)
target_include_directories(PosCoreEntities PUBLIC .)
target_link_libraries(PosCoreEntities PUBLIC Qt6::Core)

# --- Database Module ---
# This remains the single source of truth for database logic
add_library(PosCoreDatabase
    databasemanager.h databasemanager.cpp
    universalfilterproxy.h universalfilterproxy.cpp
    basemodel.h basemodel.cpp
    menutablemodel.h menutablemodel.cpp
    inventorymodel.h inventorymodel.cpp
    usermodel.h usermodel.cpp
    databaseseeder.h databaseseeder.cpp
    ordermodel.h ordermodel.cpp
    menucategorymodel.h menucategorymodel.cpp
    paymentmodel.h paymentmodel.cpp
    sqlitemigrations.h
    producttablemodel.h producttablemodel.cpp
    ordertablemodel.h ordertablemodel.cpp

)
# Use relative paths to the project root
set(SQL_MIGRATIONS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/sql/migrations/000_init.up.sql"
)

file(GLOB_RECURSE SQL_MIGRATIONS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/sql/migrations/*.sql"
)


# Step 1: Set resource alias for each file
set(SQL_RESOURCES)
foreach(sqlfile IN LISTS SQL_MIGRATIONS)
    get_filename_component(fname ${sqlfile} NAME) # get file name
    # Set alias to relative path within resource
    set(SQL_RESOURCES "${SQL_RESOURCES};${sqlfile}")
    set_property(SOURCE ${sqlfile} PROPERTY QT_RESOURCE_ALIAS "migrations/${fname}")
endforeach()

# Step 2: Add Qt resource
qt_add_resources(PosCoreDatabase POS_DB_MIGRATION_RESOURCES
    PREFIX "/sql"
    FILES ${SQL_RESOURCES}
)

target_include_directories(PosCoreDatabase PUBLIC .) # Expose current dir for headers
target_link_libraries(PosCoreDatabase
    PUBLIC
        Qt6::Sql
        PosCoreEntities

    PRIVATE
        Qt6::Core
)





