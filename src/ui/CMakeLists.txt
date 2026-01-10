qt_add_qml_module(POS_UI
    STATIC
    URI POS.UI
    VERSION 1.0

    SOURCES
       views/salesview.h views/salesview.cpp
       views/ordersview.h views/ordersview.cpp
       views/orderdetailview.h views/orderdetailview.cpp
       views/categoryview.h views/categoryview.cpp
       views/menuview.h views/menuview.cpp
       views/inventoryview.h views/inventoryview.cpp
       views/userview.h views/userview.cpp
       payment/paymentcontroller.h payment/paymentcontroller.cpp
    QML_FILES
        OrdersScreen.qml
        Main.qml
        SalesScreen.qml
        Card.qml
        OrderDetailsScreen.qml
        PaymentPage.qml

        # setup pages
        SetupScreen.qml
        MenuSetupScreen.qml
        TableSetup.qml
        ThemeSetup.qml

        # inventory management
        inventory/InventoryManagement.qml
        inventory/InventoryContextMenu.qml
        inventory/StockDialog.qml
        inventory/InventoryDetails.qml

        # sales
        sales/ModifierDialog.qml

        UserManagement.qml
        LoginScreen.qml

        SideBar.qml

        # dialogs
        AddItemDialog.qml
        ConfirmDialog.qml

        # payment
        payment/PaymentDialog.qml

    RESOURCES
        "assets/icons/default.png"
        "assets/images/bg.png"
)


target_include_directories(POS_UI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/views")
target_include_directories(POS_UI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/payment")

target_link_libraries(POS_UI
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        PosCoreEntities
        PosCoreDatabase
        PosPayment
)
