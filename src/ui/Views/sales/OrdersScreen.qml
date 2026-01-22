import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"

    // property OrdersView ordsModel: null
    // property SalesView cModel: null

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
                onClicked: if (root.ordsModel) root.ordsModel.loadOrders()
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 15 // Adds clean gap between the two elements

            TextField {
                id: searchBar
                placeholderText: "Search inventory..."
                Layout.fillWidth: true
                // Set a fixed height for uniformity
                Layout.preferredHeight: 45
                color: "white"
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 15

                // --- LOGIC: Connect to the UniversalFilterProxy ---
                onTextChanged: ordsModel.proxy.searchString = text
                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    radius: 8
                    border.color: searchBar.activeFocus ? "#c0392b" : Qt.rgba(1, 1, 1, 0.2)
                }
            }
            // Near your Search Bar RowLayout
            // ComboBox {
            //     id: filterStatus
            //     model: ["All", "PAID", "UNPAID", "OPEN"]
            //     Layout.preferredHeight: 45
            //     Layout.preferredWidth: 150
            //     onCurrentIndexChanged: {
            //         if (root.ordsModel) {
            //                     // Mapping UI index to Proxy FilterMode or CategoryFilter
            //                     root.ordsModel.proxy.categoryFilter = currentText === "All" ? "All" : currentText
            //                 }
            //         }
            // }
            // Button {
            //     id: addBtn
            //     text: "+ Add Stock"
            //     visible: inventoryPage.isManager
            //     // Match the height of the TextField
            //     Layout.preferredHeight: 45
            //     Layout.preferredWidth: 140

            //     contentItem: Text {
            //         text: addBtn.text
            //         color: "white"
            //         horizontalAlignment: Text.AlignHCenter
            //         verticalAlignment: Text.AlignVCenter
            //         font.bold: true
            //     }

            //     background: Rectangle {
            //         color: addBtn.pressed ? "#a03023" : (addBtn.hovered ? "#d35400" : "#c0392b")
            //         radius: 8
            //     }

            //     onClicked: {
            //         stockLoader.active = true
            //         stockLoader.item.openForAdd()
            //     }
            // }
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

    Component.onCompleted: if (root.ordsModel) root.ordsModel.loadOrders()
}
