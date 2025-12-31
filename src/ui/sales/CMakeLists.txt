qt_add_qml_module(POS_Sales
    STATIC
    URI POS.Sales
    VERSION 1.0
    SOURCES
       salesmodel.h salesmodel.cpp

    QML_FILES
        SalesScreen.qml
        Main.qml
        MenuScreen.qml
        Card.qml
        OrderDetailsScreen.qml
    RESOURCES
        "../assets/icons/default.png"
)


target_link_libraries(POS_Sales
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        core_entities  # Link to the entities/order logic
        order          # Link to the order service
)
