import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"

    property OrdersView ordsModel: null
    property SalesView cModel: null

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 15

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 2
                Text { text: "Order History"; font.pixelSize: 28; font.bold: true; color: "white" }
                Text { text: "View and manage past transactions"; font.pixelSize: 14; color: "#bdc3c7" }
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "↻ Refresh List"
                onClicked: if (root.ordsModel) root.ordsModel.loadOrderHistory()
            }
        }

        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4)
            radius: 12
            border.color: Qt.rgba(255, 255, 255, 0.1)
            clip: true

            ListView {
                id: historyListView
                anchors.fill: parent; anchors.margins: 1
                model: root.ordsModel
                headerPositioning: ListView.OverlayHeader
                header: Rectangle {
                    width: historyListView.width; height: 45; color: Qt.rgba(1, 1, 1, 0.1); z: 2
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 15
                        Text { text: "Order Reference"; font.bold: true; color: "white"; Layout.fillWidth: true }
                        Text { text: "Date & Time"; font.bold: true; color: "white"; Layout.preferredWidth: 150 }
                    }
                }

                delegate: ItemDelegate {
                    width: historyListView.width; height: 60
                    background: Rectangle {
                        color: hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Qt.rgba(1, 1, 1, 0.05) }
                    }
                    contentItem: RowLayout {
                        anchors.fill: parent; anchors.margins: 15
                        Text { text: model.displayTitle; color: "white"; font.bold: true; Layout.fillWidth: true }
                        Text { text: model.date; color: "#bdc3c7"; Layout.preferredWidth: 150 }
                    }
                    onClicked: {
                        globalOrderDetailModel.loadOrder(model.orderId)
                        contentStack.push(orderDetailsView, { "currentOrderId": model.orderId })
                    }
                }
            }
        }
    }

    Component.onCompleted: if (root.ordsModel) root.ordsModel.loadOrderHistory()
}
