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

    property QtObject theme: spicyRed
    // --- 2. DEFINE THE spicyRed DARK THEME ---
    QtObject {
            id: spicyRed
            property color background: "transparent"
            property color surface: Qt.rgba(1, 1, 1, 0.08)
            property color surfaceHighlight: Qt.rgba(1, 1, 1, 0.18)
            property color accent: "#3498db"
            property color success: "#2ecc71"
            property color danger: "#e74c3c"
            property color textMain: "#ffffff"
            property color textSecondary: "#95a5a6"
            property color border: Qt.rgba(1, 1, 1, 0.15)
            property color sidePanelBg: Qt.rgba(0, 0, 0, 0.25)

            // --- Card Specific Props ---
            property real cardOpacity: 0.9
            property real cardIconOpacity: 0.9
            property color cardBorderFocused: "#3498db"
            property int cardRadius: 12
        }

    // --- 3. DEFINE THE WHITE THEME ---
        QtObject {
            id: whiteTheme
            property color background: "#f5f6fa"
            property color surface: "#ffffff"
            property color surfaceHighlight: "#f1f2f6"
            property color accent: "#2980b9"
            property color success: "#27ae60"
            property color danger: "#c0392b"
            property color textMain: "#2f3640"
            property color textSecondary: "#7f8c8d"
            property color border: "#dcdde1"
            property color sidePanelBg: "#ebedf0"
            // --- Card Specific Props ---
            property real cardOpacity: 1.0
            property real cardIconOpacity: 1.0
            property color cardBorderFocused: "#2980b9"
            property int cardRadius: 8
        }
    // Model Logic
    MenuViewController {
        id: globalCartModel
    }
    // SalesView { id: globalCartModel }
    // OrdersView { id: globalOrdersModel }
    // OrderDetailView { id: globalOrderDetailModel }
    // UserView { id: globalUserModel }
    // InventoryView { id: globalInventoryModel }

    property bool sidebarCollapsed: width < 900

    StackView {
        id: rootStack
        anchors.fill: parent
        initialItem: mainLayout
    }

    // Component {
    //     id: loginScreenComponent
    //     LoginScreen { onLoginSuccess: rootStack.replace(mainLayout) }
    // }

    Component {
        id: mainLayout
        Rectangle {
            id: mainBackground
            color: "#1a0505"
            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                            id: backgroundImage
                            source: "qrc:/qt/qml/POS/UI/assets/images/bg-white.png"
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
                        // { name: "Order History", view: ordersView, icon: "📋" },
                        {
                            name: "System Setup",
                            icon: "⚙",
                            subItems: [
                                // { name: "Menu Setup", view: menuSetupView },
                                { name: "Table Setup", view: tableSetupView },
                                { name: "Theme Setup", view: themeSetupView }
                            ]
                        },
                        { name: "User Management", view: userMgmtView, icon: "👤" },
                        { name: "Inventory", view: inventoryMgmtView, icon: "📦" }
                    ]
                }

               // fix:new model // Ensure the sub-views are defined in the contentStack area
                // Component { id: menuSetupView; MenuSetupScreen { objectName: "Menu Setup" } }
                // Component { id: tableSetupView; TableSetup { objectName: "Table Setup" } }
                // Component { id: themeSetupView; ThemeSetup { objectName: "Theme Setup" } }

                // --- MAIN VIEWPORT ---
                // Now stretches edge-to-edge without a top bar
                StackView {
                    id: contentStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    initialItem: salesView

                    Component { id: salesView; SalesScreen {
                            // salesModel: globalCartModel
                        } }
                    // Component { id: ordersView; OrdersScreen { ordsModel: globalOrdersModel } }
                    // Component { id: orderDetailsView; OrderDetailsScreen { detailsModel: globalOrderDetailModel } }
                    // Component { id: setupView; SetupScreen { } }
                    // Component { id: userMgmtView; UserManagement { staffModel: globalUserModel } }
                    // Component {
                    //     id: inventoryMgmtView;
                    //     InventoryManagement {
                    //         // Use the global ID you defined at the top of Main.qml
                    //         invModel: globalInventoryModel
                    //         usrModel: globalUserModel
                    //     }
                    // }
                    // Smooth Fade transition for cleaner feel
                    replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 } }
                    replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 150 } }
                }
            }
        }
    }
}
