import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: dashboardRoot
    anchors.fill: parent
    color: "transparent"
    SalesViewController {
        id: globalSalesController

        // When any part of the app modifies the DB,
        // this instance will emit kitchenDataChanged
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0


        // --- TOP NAVIGATION BAR ---
        Rectangle {
            id: navHeader
            Layout.fillWidth: true
            Layout.preferredHeight: 65
            color: "transparent" // Slightly lighter dark for the header

            // Bottom border matching the sidebar style
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Qt.rgba(1, 1, 1, 0.1)
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 25
                spacing: 20

                // Internal component to keep button styles identical
                component NavButton : Button {
                    property int targetIndex: 0
                    Layout.preferredWidth: 150
                    Layout.fillHeight: true
                    flat: true

                    contentItem: Text {
                        text: parent.text
                        color: mainViewStack.currentIndex === parent.targetIndex ? "#2ecc71" : "#95a5a6"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent"

                        // Selection Indicator (Bottom Line)
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width * 0.7
                            anchors.horizontalCenter: parent.horizontalCenter
                            height: 3
                            color: "#2ecc71"
                            visible: mainViewStack.currentIndex === parent.targetIndex
                        }
                    }
                    onClicked: mainViewStack.currentIndex = targetIndex
                }

                NavButton { text: "MENU VIEW"; targetIndex: 0 }
                NavButton { text: "ORDER HISTORY"; targetIndex: 1 }
                NavButton { text: "KITCHEN VIEW"; targetIndex: 2 }

                Item { Layout.fillWidth: true } // Spacer to push buttons to the left
            }
        }

        // --- MAIN CONTENT AREA ---
        StackLayout {
            id: mainViewStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0 // Default to SalesScreen

            SalesScreen {
                // Pass the shared controller to the screen
                _salesModel: globalSalesController
            }

            OrdersScreen {
                _salesModel: globalSalesController
            }

            KitchenView {
                _salesModel: globalSalesController
            }
        }
    }
}
