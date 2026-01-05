import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    title: "Restaurant POS"
    // --- 1. Separate Model Instances ---
    // Handles the active shopping cart (Menu Page)
    SalesView {
        id: globalCartModel
    }

    // Handles database lookups (History Page)
    HistoryView {
        id: globalHistoryModel
    }
    OrderDetailView {
        id: globalOrderDetailModel
    }
    UserView{ id: globalUserModel }
        InventoryView { id: globalInventoryModel }

    property bool isFullScreen: false
        StackView {
                id: rootStack
                anchors.fill: parent

                // Use the separate file as initial item
                initialItem: loginScreenComponent

                replaceEnter: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 400 }
                }
            }

            // Component wrapper for the external file
            Component {
                id: loginScreenComponent
                LoginScreen {
                    onLoginSuccess: rootStack.replace(mainLayout)
                }
            }
            Component {
                    id: mainLayout
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Sidebar ---
        Rectangle {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: window.isFullScreen ? 0 : 200
            color: "#2c3e50"
            clip: true

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15
                opacity: window.isFullScreen ? 0 : 1

                Label {
                    text: "POS SYSTEM"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Button {
                    text: "Menu / Ordering"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(menuView)
                }

                Button {
                    text: "Order History"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(salesView)
                }
                Button {
                    text: "⚙ System Setup"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(setupView)
                }
                Button {
                    text: "👤 User Management"
                    Layout.fillWidth: true
                    // Only show if the current session is a manager
                    // visible: globalUserModel.isAdmin()
                    onClicked: contentStack.replace(userMgmtView)
                }
                Button {
                    text: "📦 Inventory"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(inventoryView)
                }

                Item { Layout.fillHeight: true }
            }
        }

        // --- Main Content Area ---
        StackView {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            initialItem: menuView

            // 2. Assign the appropriate model to each view
                Component { id: menuView; MenuScreen { salesModel: globalCartModel } }
                Component { id: salesView; OrdersScreen { hModel: globalHistoryModel } }
                Component { id: orderDetailsView; OrderDetailsScreen { detailsModel: globalOrderDetailModel } }
                Component { id: setupView; SetupScreen { } }
                Component { id: userMgmtView; UserManagement { staffModel: globalUserModel } }
                Component { id: inventoryView; InventoryManagement { invModel: globalInventoryModel; usrModel: globalUserModel } }

            // Slide Transitions
            replaceEnter: Transition {
                PropertyAnimation {
                    property: "x"
                    from: contentStack.width
                    to: 0
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }
            replaceExit: Transition {
                PropertyAnimation {
                    property: "x"
                    from: 0
                    to: -contentStack.width
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}
}
