# --- Payment Module ---
# This remains the single source of truth for database logic
add_library(PosPayment
    payment.h       payment.cpp
    cashpayment.h   cashpayment.cpp
    mpesapayment.h  mpesapayment.cpp
    payment_enums.h
)
target_include_directories(PosPayment PUBLIC .) # Expose current dir for headers
target_link_libraries(PosPayment
    PUBLIC
        Qt6::Sql
        Qt6::Network
        PosCoreDatabase

    PRIVATE
        Qt6::Core
)

