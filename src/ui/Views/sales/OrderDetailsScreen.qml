import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    property string currentOrderId: ""
    property SalesViewController _salesModel

    // Mimic Kitchen View logic: Fetch raw data into this property
    property var orderData: null

    color: "transparent"

    // Helper to find the specific order from the list
    function refresh() {
        if (!_salesModel || currentOrderId === "") return;
        let allOrders = _salesModel.loadOrders();
        for (let i = 0; i < allOrders.length; i++) {
            if (allOrders[i].orderId === currentOrderId) {
                root.orderData = allOrders[i];
                break;
            }
        }
    }

    Connections {
        target: _salesModel
        function onKitchenDataChanged() { refresh(); }
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 25; spacing: 20

        // --- NAVIGATION HEADER (Original Design) ---
        RowLayout {
            spacing: 15
            Button {
                text: "←"
                flat: true; font.pixelSize: 24; palette.buttonText: "white"
                onClicked: contentStack.pop()
            }

            Text {
                text: "Order Details #" + currentOrderId.substring(0, 8)
                font.pixelSize: 24; font.bold: true; color: "white"
            }
            Item { Layout.fillWidth: true }
        }

        // --- ITEMS LIST (Original Design) ---
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4); radius: 12; border.color: Qt.rgba(255, 255, 255, 0.1); clip: true

            ListView {
                id: detailsList
                anchors.fill: parent; anchors.margins: 1
                // Use the inner itemModel from our orderData
                model: root.orderData ? root.orderData.itemModel : []
                spacing: 0

                delegate: ItemDelegate {
                    width: detailsList.width; height: 70

                    background: Rectangle {
                        color: hovered ? Qt.rgba(255, 255, 255, 0.05) : "transparent"
                        Rectangle {
                            anchors.bottom: parent.bottom; width: parent.width; height: 1
                            color: Qt.rgba(255, 255, 255, 0.05)
                        }
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent; anchors.margins: 15
                        ColumnLayout {
                            Layout.fillWidth: true
                            Text {
                                text: modelData.name // Use modelData for JS arrays
                                color: "white"; font.bold: true; font.pixelSize: 16
                            }
                            Text {
                                text: (modelData.status || "ordered").toUpperCase()
                                color: modelData.status === "served" ? "#2ecc71" : "#f1c40f"
                                font.pixelSize: 11; font.bold: true
                            }
                        }

                        // --- DYNAMIC STATUS BUTTON ---
                        Button {
                            id: statusBtn
                            Layout.preferredWidth: 110
                            Layout.preferredHeight: 40
                            visible: modelData.status !== "served"

                            contentItem: Text {
                                text: modelData.status === "ordered" ? "PREPARE" : "SERVE"
                                color: "#3498db"; font.bold: true; font.pixelSize: 11
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                let nextStatus = (modelData.status === "ordered") ? "preparing" : "served";
                                _salesModel.updateItemStatus(modelData.itemId, nextStatus);
                                // Refresh is handled by the Connections block
                            }
                        }
                    }
                }
            }
        }

        // --- ACTION BUTTONS (Original Design) ---
        RowLayout {
            Layout.fillWidth: true; spacing: 15
            Button { text: "Print Kitchen"; Layout.fillWidth: true; Layout.preferredHeight: 50 }
            Button { text: "Print Bill"; highlighted: true; Layout.fillWidth: true; Layout.preferredHeight: 50 }
            Button {
                text: "Proceed to Payment >"
                highlighted: true; palette.button: "#2ecc71"
                Layout.fillWidth: true; Layout.preferredHeight: 50
            }
        }
    }

    Component.onCompleted: refresh()
}
