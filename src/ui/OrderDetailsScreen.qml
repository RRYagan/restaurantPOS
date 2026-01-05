import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    property string currentOrderId: ""
    onCurrentOrderIdChanged: console.log("New Order ID received in Details Screen:", currentOrderId)
    property OrderDetailView detailsModel: null
    color: "#f8f9fa"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // Header with Back Button
        // Header with Back Button and Locked Status
        RowLayout {
            Button {
                text: "← Back"
                onClicked: contentStack.pop()
            }

            // New "Locked" Indicator
            Rectangle {
                color: "#ffebee"
                border.color: "#ef5350"
                radius: 4
                implicitWidth: 80
                implicitHeight: 30

                Row {
                    anchors.centerIn: parent
                    spacing: 5
                    Text { text: "🔒"; font.pixelSize: 14 }
                    Text {
                        text: "LOCKED"
                        color: "#c62828"
                        font.bold: true
                        font.pixelSize: 11
                    }
                }
            }

            Text {
                text: "Order Details #" + currentOrderId
                font.pixelSize: 22
                font.bold: true
            }
        }
        // 1) Order Breakdown List
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: "#ddd"
            radius: 4

            ListView {
                anchors.fill: parent
                anchors.margins: 10
                Layout.fillWidth: true
                model: detailsModel
                clip: true
                header: RowLayout {
                    width: parent.width
                    Text { text: "Item"; font.bold: true; Layout.fillWidth: true }
                    Text { text: "Qty"; font.bold: true; Layout.preferredWidth: 40 }
                    Text { text: "Price"; font.bold: true; Layout.preferredWidth: 80 }
                }
                delegate: ItemDelegate {
                    width: parent.width
                    contentItem: RowLayout {
                        Text { text: model.name; Layout.fillWidth: true }
                        Text { text: model.quantity.toString(); Layout.preferredWidth: 40 }
                        Text { text: model.price.formatted; Layout.preferredWidth: 80 }
                    }
                }
            }
        }

        // 2) Action Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            Button {
                text: "Print Kitchen Receipt"
                Layout.fillWidth: true
                onClicked: console.log("Printing Kitchen Copy for Order:", currentOrderId)
            }

            Button {
                text: "Print Customer Receipt"
                highlighted: true
                Layout.fillWidth: true
                onClicked: console.log("Printing Customer Copy for Order:", currentOrderId)
            }
            Button {
                    text: "Proceed to Payment >"
                    highlighted: true
                    palette.button: "#2ecc71"
                    Layout.fillWidth: true
                    onClicked: contentStack.push("PaymentPage.qml", {
                        "salesModel": salesModel,
                        "orderId": currentOrderId
                    })
                }
        }
    }

    Component.onCompleted: {
        if (currentOrderId !== "" && detailsModel !== null) {
            detailsModel.loadOrder(currentOrderId); // Use the new loadOrder method
        } else {
            console.error("OrderDetailsScreen: salesModel is null or OrderID is invalid. " +
                                  "ID: '" + currentOrderId + "', Model: " + historyModel);
            }
    }
}
