import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    title: "Premium POS System"

    // Model Logic
    SalesView { id: globalCartModel }
    OrdersView { id: globalOrdersModel }
    OrderDetailView { id: globalOrderDetailModel }
    UserView { id: globalUserModel }
    InventoryView { id: globalInventoryModel }

    property bool sidebarCollapsed: width < 900

    StackView {
        id: rootStack
        anchors.fill: parent
        initialItem: loginScreenComponent
    }

    Component {
        id: loginScreenComponent
        LoginScreen { onLoginSuccess: rootStack.replace(mainLayout) }
    }

    Component {
        id: mainLayout
        Rectangle {
            id: mainBackground
            color: "#1a0505"
            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                            id: backgroundImage
                            source: "qrc:/qt/qml/POS/UI/assets/images/bg.png"
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            opacity: 0.3 // Adjust opacity to ensure UI text remains readable
                            asynchronous: true
                        }
            RowLayout {
                anchors.fill: parent
                spacing: 0

                SideBar {
                    id: sideNav
                    isCollapsed: window.sidebarCollapsed
                    targetStack: contentStack
                    Layout.rightMargin: 15

                    menuModel: [
                        { name: "Menu / Ordering", view: salesView, icon: "🍴" },
                        { name: "Order History", view: ordersView, icon: "📋" },
                        {
                            name: "System Setup",
                            icon: "⚙",
                            subItems: [
                                { name: "Menu Setup", view: menuSetupView },
                                { name: "Table Setup", view: tableSetupView },
                                { name: "Theme Setup", view: themeSetupView }
                            ]
                        },
                        { name: "User Management", view: userMgmtView, icon: "👤" },
                        { name: "Inventory", view: inventoryView, icon: "📦" }
                    ]
                }

                // Ensure the sub-views are defined in the contentStack area
                Component { id: menuSetupView; MenuSetupScreen { objectName: "Menu Setup" } }
                Component { id: tableSetupView; TableSetup { objectName: "Table Setup" } }
                Component { id: themeSetupView; ThemeSetup { objectName: "Theme Setup" } }

                // --- MAIN VIEWPORT ---
                // Now stretches edge-to-edge without a top bar
                StackView {
                    id: contentStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    initialItem: salesView

                    Component { id: salesView; SalesScreen { salesModel: globalCartModel } }
                    Component { id: ordersView; OrdersScreen { ordsModel: globalOrdersModel } }
                    Component { id: orderDetailsView; OrderDetailsScreen { detailsModel: globalOrderDetailModel } }
                    Component { id: setupView; SetupScreen { } }
                    Component { id: userMgmtView; UserManagement { staffModel: globalUserModel } }
                    Component { id: inventoryView; InventoryManagement { invModel: globalInventoryModel.proxy; usrModel: globalUserModel } }

                    // Smooth Fade transition for cleaner feel
                    replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 } }
                    replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 150 } }
                }
            }
        }
    }
}
