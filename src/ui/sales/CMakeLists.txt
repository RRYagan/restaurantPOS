qt_add_qml_module(POS_Sales
    STATIC
    URI POS.Sales
    VERSION 1.0
    SOURCES
       models/salesmodel.h models/salesmodel.cpp
       models/historymodel.h models/historymodel.cpp
       models/orderdetailmodel.h models/orderdetailmodel.cpp
       models/categorymodel.h models/categorymodel.cpp
       models/menumodel.h models/menumodel.cpp

    QML_FILES
        SalesScreen.qml
        Main.qml
        MenuScreen.qml
        Card.qml
        OrderDetailsScreen.qml
        PaymentPage.qml
        SetupScreen.qml
        MenuSetup.qml
        TableSetup.qml
        ThemeSetup.qml
    RESOURCES
        "../assets/icons/default.png"
)

target_include_directories(POS_Sales PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/models")
target_link_libraries(POS_Sales
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        core_entities  # Link to the entities/order logic
        # order          # Link to the order service
        database
)
