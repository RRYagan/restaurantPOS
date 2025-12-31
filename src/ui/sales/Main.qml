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
    SalesModel {
            id: globalSalesModel
        }
    // Property to control sidebar and split-screen state
    property bool isFullScreen: false

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Sidebar ---
        Rectangle {
            id: sidebar
            Layout.fillHeight: true
            // Smoothly collapses to 0 width
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
                    text: "Menu Management"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(menuView)
                }

                Button {
                    text: "Sales View"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(salesView)
                }


                Item { Layout.fillHeight: true }
            }
        }

        StackView {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            initialItem: menuView

            Component { id: menuView; MenuScreen {salesModel: globalSalesModel} }
            Component { id: salesView;
                SalesScreen {salesModel: globalSalesModel}
            }
            Component {
                id: orderDetailsView
                OrderDetailsScreen { salesModel: globalSalesModel }
            }

            // This defines the smooth slide transition
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
