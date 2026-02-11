# --- Payment Module ---
# This remains the single source of truth for database logic
add_library(PosFinance
    financemodel.h financemodel.cpp

)
target_include_directories(PosFinance PUBLIC .) # Expose current dir for headers
target_link_libraries(PosFinance
    PUBLIC
        Qt6::Sql
        Qt6::Network
        PosCoreDatabase

    PRIVATE
        Qt6::Core
)

