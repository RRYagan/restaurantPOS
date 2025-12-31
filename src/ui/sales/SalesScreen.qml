import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: root
    color: "#f8f9fa"
    property SalesModel salesModel: null

    StackView.onActivated: {
            if (salesModel) {
                salesModel.loadOrderHistory(); // Refresh list every time we navigate back here
            }
        }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "Order History"
                font.pixelSize: 24
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "Refresh History"
                onClicked: salesModel.loadOrderHistory()
            }
        }

        ListView {
            id: orderHistoryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: salesModel

            delegate: ItemDelegate {
                width: orderHistoryList.width
                contentItem: RowLayout {
                    ColumnLayout {
                        Text {
                            text: model.name // Displays "Order #ID (Table X)"
                            font.bold: true
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: "Status: OPEN" // Based on your 'status' column default
                        color: "#2980b9"
                    }
                }
                onClicked: {
                    contentStack.push("OrderDetailsScreen.qml", {
                        "currentOrderId": model.itemId,
                        "salesModel": salesModel
                    })
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Text {
                font.pixelSize: 16
                text: "Total Orders: " + salesModel.rowCount()
            }
        }
    }

    // Load history automatically when the screen is navigated to
    Component.onCompleted: {
        if (salesModel) {
            salesModel.loadOrderHistory()
        }
    }
}
