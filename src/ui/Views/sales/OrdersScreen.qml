import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"

    // Instantiate the controller
    SalesViewController {
        id: _salesModel

        // When the backend signals a change, refresh our local list
        onKitchenDataChanged: root.refreshOrders()
    }

    // Local model to drive the ListView
    ListModel { id: ordersHistoryModel }

    // Helper function to fetch data from C++ and populate the UI
    function refreshOrders() {
        var data = _salesModel.loadOrders();
        ordersHistoryModel.clear();
        for (var i = 0; i < data.length; i++) {
            ordersHistoryModel.append(data[i]);
        }
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 15

        // --- HEADER SECTION ---
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
                onClicked: root.refreshOrders()
            }
        }

        // --- SEARCH & FILTER SECTION ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            TextField {
                id: searchBar
                placeholderText: "Search by Table or Reference..."
                Layout.fillWidth: true
                Layout.preferredHeight: 45
                color: "white"
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 15

                // Local filtering logic if not using a C++ Proxy
                onTextChanged: {
                    // You can call a backend search or filter the local ListModel here
                }

                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    radius: 8
                    border.color: searchBar.activeFocus ? "#2ecc71" : Qt.rgba(1, 1, 1, 0.2)
                }
            }

            ComboBox {
                id: filterStatus
                model: ["All Status", "open", "closed", "voided"]
                Layout.preferredHeight: 45
                Layout.preferredWidth: 150
            }
        }

        // --- DATA TABLE ---
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4)
            radius: 12
            border.color: Qt.rgba(255, 255, 255, 0.1)
            clip: true

            ListView {
                id: historyListView
                anchors.fill: parent; anchors.margins: 1
                model: ordersHistoryModel
                headerPositioning: ListView.OverlayHeader

                header: Rectangle {
                    width: historyListView.width; height: 45; color: "#1a1a1a"; z: 2
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 15
                        Text { text: "Order Reference"; font.bold: true; color: "white"; Layout.fillWidth: true }
                        Text { text: "Status"; font.bold: true; color: "white"; Layout.preferredWidth: 100 }
                        Text { text: "Date & Time"; font.bold: true; color: "white"; Layout.preferredWidth: 150 }
                    }
                }

                delegate: ItemDelegate {
                    width: historyListView.width; height: 60

                    background: Rectangle {
                        color: hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
                        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Qt.rgba(1, 1, 1, 0.05) }
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent; anchors.margins: 15

                        // Reference & Table
                        Text {
                            text: model.displayTitle
                            color: "white"; font.bold: true; Layout.fillWidth: true
                        }

                        // Status Badge
                        Rectangle {
                            Layout.preferredWidth: 80; Layout.preferredHeight: 24
                            radius: 12
                            color: model.status === "open" ? Qt.rgba(46, 204, 113, 0.2) : Qt.rgba(149, 165, 166, 0.2)
                            border.color: model.status === "open" ? "#2ecc71" : "#95a5a6"

                            Text {
                                anchors.centerIn: parent
                                text: model.status.toUpperCase()
                                font.pixelSize: 10; font.bold: true
                                color: parent.border.color
                            }
                        }

                        // Timestamp
                        Text {
                            text: model.date; color: "#bdc3c7";
                            Layout.preferredWidth: 150; horizontalAlignment: Text.AlignRight
                        }
                    }

                    onClicked: {
                        // 1. Check if the IDs are accessible.
                        // If they are in main.qml, you might need to use 'rootWindow.contentStack'
                        // or whatever ID you gave your top-level window.
                        if (typeof globalOrderDetailModel !== "undefined" && typeof contentStack !== "undefined") {

                            // Load the order into the C++ detail model
                            globalOrderDetailModel.loadOrder(model.orderId);

                            // Push the view. Ensure 'orderDetailsView' is a defined Component
                            contentStack.push(orderDetailsView, {
                                "currentOrderId": model.orderId,
                                "detailsModel": globalOrderDetailModel // Explicitly pass the model if needed
                            });
                        } else {
                            console.error("Navigation Error: IDs not found in current scope.");
                            // Fallback: Try to find them via the root object if you have one
                            // rootApp.contentStack.push(...)
                        }
                    }
                }
            }
        }
    }

    // Initial load
    Component.onCompleted: root.refreshOrders()
}
