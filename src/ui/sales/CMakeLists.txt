qt_add_qml_module(POS_Sales
    STATIC
    URI POS.Sales
    VERSION 1.0
    SOURCES
        # List the headers and sources
        models/categorymodel.cpp models/categorymodel.h
        models/historymodel.cpp  models/historymodel.h
        models/menumodel.cpp     models/menumodel.h
        models/orderdetailmodel.cpp models/orderdetailmodel.h
        models/salesmodel.cpp    models/salesmodel.h
    QML_FILES
        SalesScreen.qml
        Main.qml
        MenuScreen.qml
        Card.qml
        OrderDetailsScreen.qml
        PaymentPage.qml
        SetupScreen.qml
    RESOURCES
        "../assets/icons/default.png"
)

# CRITICAL: This allows the generated registration file to find your headers
target_include_directories(POS_Sales PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/models"
)

target_link_libraries(POS_Sales
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        database
        core_entities
)
