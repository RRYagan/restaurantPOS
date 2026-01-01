import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    title: "Restaurant POS"

    // --- 1. Separate Model Instances ---
    // Handles the active shopping cart (Menu Page)
    SalesModel {
        id: globalCartModel
    }

    // Handles database lookups (History Page)
    HistoryModel {
        id: globalHistoryModel
    }

    property bool isFullScreen: false

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
            Component {
                id: menuView
                MenuScreen {
                    salesModel: globalCartModel
                }
            }

            Component {
                id: salesView
                SalesScreen {
                    // HistoryScreen uses the historyModel to avoid flickering the cart
                    hModel: globalHistoryModel
                    cModel: globalCartModel // We still pass cartModel if we need detail lookups
                }
            }

            Component {
                id: orderDetailsView
                OrderDetailsScreen {
                    // We use the cartModel's detail view logic here
                    salesModel: globalCartModel
                }
            }

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
