qt_add_qml_module(POS_UI
    STATIC
    URI POS.UI
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/POS/UI"

    SOURCES
       Controllers/salesview.h Controllers/salesview.cpp
       # controllers/ordersview.h controllers/ordersview.cpp
       # controllers/orderdetailview.h controllers/orderdetailview.cpp
       # controllers/categoryview.h controllers/categoryview.cpp
       # controllers/menuviewcontroller.h controllers/menuviewcontroller.cpp
       Controllers/inventoryviewcontroller.h Controllers/inventoryviewcontroller.cpp
       # controllers/userview.h controllers/userview.cpp
       Controllers/paymentcontroller.h Controllers/paymentcontroller.cpp
       Controllers/productviewcontroller.h Controllers/productviewcontroller.cpp
       # controllers/orderviewcontroller.h controllers/orderviewcontroller.cpp
       Controllers/salesviewcontroller.h Controllers/salesviewcontroller.cpp

    QML_FILES
        Views/Main.qml

        # setup pages
        Views/setup/SetupScreen.qml
        Views/setup/MenuSetupView.qml
        Views/setup/SetupDashboard.qml
        Views/TableSetup.qml
        Views/ThemeSetup.qml

        # inventory management
        Views/inventory/InventoryManagement.qml
        Views/inventory/InventoryContextMenu.qml
        Views/inventory/StockDialog.qml
        Views/inventory/InventoryDetails.qml

        # sales
        Views/sales/ModifierDialog.qml
        Views/sales/OrdersScreen.qml
        Views/sales/SalesScreen.qml
        Views/sales/Card.qml
        Views/sales/OrderDetailsScreen.qml
        Views/sales/payment/PaymentPage.qml
        Views/sales/payment/PaymentDialog.qml

        Views/hr/UserManagement.qml
        Views/authentication/LoginScreen.qml

        Views/SideBar.qml

        # dialogs
        Views/AddItemDialog.qml
        Views/ConfirmDialog.qml



        Views/TestSales.qml
        Views/ProductSetupScreen.qml

    RESOURCES
        "Views/assets/icons/default.png"
        "Views/assets/images/bg.png"
        "Views/assets/images/bg-white.png"
)





# target_include_directories(POS_UI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/payment")
target_include_directories(POS_UI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Controllers")

target_link_libraries(POS_UI
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        PosCoreDatabase
        PosPayment
)
