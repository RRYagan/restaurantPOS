import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    property string currentOrderId: ""
    // property OrderDetailView detailsModel: null
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 25; spacing: 20

        RowLayout {
            spacing: 15
            Button {
                text: "←"
                flat: true
                font.pixelSize: 24
                palette.buttonText: "white"
                onClicked: contentStack.pop()
            }

            Rectangle {
                color: Qt.rgba(1, 0, 0, 0.2)
                border.color: "#ef5350"
                radius: 4
                implicitWidth: 90; implicitHeight: 32
                Row {
                    anchors.centerIn: parent; spacing: 5
                    Text { text: "🔒"; font.pixelSize: 14 }
                    Text { text: "LOCKED"; color: "white"; font.bold: true; font.pixelSize: 11 }
                }
            }

            Text { text: "Order Details #" + currentOrderId; font.pixelSize: 24; font.bold: true; color: "white" }
            Item { Layout.fillWidth: true }
        }

        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4); radius: 12; border.color: Qt.rgba(255, 255, 255, 0.1)
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 20; spacing: 10
                RowLayout {
                    Text { text: "Item"; font.bold: true; color: "#bdc3c7"; Layout.fillWidth: true }
                    Text { text: "Qty"; font.bold: true; color: "#bdc3c7"; Layout.preferredWidth: 60; horizontalAlignment: Text.AlignHCenter }
                    Text { text: "Price"; font.bold: true; color: "#bdc3c7"; Layout.preferredWidth: 100; horizontalAlignment: Text.AlignRight }
                }
                Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(1, 1, 1, 0.1) }
                ListView {
                    id: detailsList; Layout.fillWidth: true; Layout.fillHeight: true; model: detailsModel; clip: true; spacing: 5
                    delegate: ItemDelegate {
                        width: detailsList.width; height: 45
                        background: Rectangle { color: hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent" }
                        contentItem: RowLayout {
                            Text { text: model.name; color: "white"; font.pixelSize: 15; Layout.fillWidth: true }
                            Text { text: model.quantity.toString(); color: "white"; Layout.preferredWidth: 60; horizontalAlignment: Text.AlignHCenter }
                            Text { text: model.price ? model.price.formatted : "0.00"; color: "#2ecc71"; font.bold: true; Layout.preferredWidth: 100; horizontalAlignment: Text.AlignRight }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true; spacing: 15
            Button { text: "Print Kitchen Receipt"; Layout.fillWidth: true; Layout.preferredHeight: 50 }
            Button { text: "Print Customer Receipt"; highlighted: true; Layout.fillWidth: true; Layout.preferredHeight: 50 }
            Button { text: "Proceed to Payment >"; highlighted: true; palette.button: "#2ecc71"; Layout.fillWidth: true; Layout.preferredHeight: 50 }
        }
    }

    Component.onCompleted: if (currentOrderId !== "" && detailsModel !== null) detailsModel.loadOrder(currentOrderId)
}
