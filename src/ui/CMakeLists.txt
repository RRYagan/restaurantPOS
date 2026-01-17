qt_add_qml_module(POS_UI
    STATIC
    URI POS.UI
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/POS/UI"

    SOURCES
       # controllers/salesview.h controllers/salesview.cpp
       # controllers/ordersview.h controllers/ordersview.cpp
       # controllers/orderdetailview.h controllers/orderdetailview.cpp
       # controllers/categoryview.h controllers/categoryview.cpp
       # controllers/menuviewcontroller.h controllers/menuviewcontroller.cpp
       controllers/inventoryviewcontroller.h controllers/inventoryviewcontroller.cpp
       # controllers/userview.h controllers/userview.cpp
       payment/paymentcontroller.h payment/paymentcontroller.cpp
       controllers/productviewcontroller.h controllers/productviewcontroller.cpp
       # controllers/orderviewcontroller.h controllers/orderviewcontroller.cpp

    QML_FILES
        OrdersScreen.qml
        Main.qml
        SalesScreen.qml
        Card.qml
        OrderDetailsScreen.qml
        PaymentPage.qml

        # setup pages
        setup/SetupScreen.qml
        setup/MenuSetupView.qml
        setup/SetupDashboard.qml
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

        TestSales.qml
        ProductSetupScreen.qml

    RESOURCES
        "assets/icons/default.png"
        "assets/images/bg.png"
        "assets/images/bg-white.png"
)





target_include_directories(POS_UI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/payment")
target_include_directories(POS_UI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/controllers")

target_link_libraries(POS_UI
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        # PosCoreEntities
        PosCoreDatabase
        PosPayment
)
