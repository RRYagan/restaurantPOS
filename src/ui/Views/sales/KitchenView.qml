import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: kitchenRoot
    color: "transparent"
    // Shared controller passed from SalesDashboard
    property SalesViewController _salesModel

    ListModel { id: kdsModel }

    // Logic to update the local list from the C++ controller [cite: 14]
    function refresh() {
        if (!_salesModel) return;
        var data = _salesModel.getKitchenQueue();
        kdsModel.clear();
        for (var i = 0; i < data.length; i++) {
            kdsModel.append(data[i]);
        }
    }

    // Listens for the kitchenDataChanged signal from the C++ controller [cite: 14]
    Connections {
        target: kitchenRoot._salesModel
        function onKitchenDataChanged() {
            console.log("KitchenView: Refreshing data...");
            refresh();
        }
    }

    Component.onCompleted: refresh()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        RowLayout {
            Layout.fillWidth: true
            Text { text: "Kitchen Queue"; font.pixelSize: 24; font.bold: true; color: "white" }
            Item { Layout.fillWidth: true }

            Button {
                text: "↻ Refresh Queue"
                onClicked: kitchenRoot.refresh()
                background: Rectangle {
                    color: parent.pressed ? "#34495e" : "#2c3e50"
                    radius: 6
                    border.color: "#2ecc71"
                }
                contentItem: Text {
                    text: parent.text
                    color: "#2ecc71"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

    GridView {
        anchors.fill: parent
        anchors.margins: 20
        cellWidth: 320; cellHeight: 400
        model: kdsModel
        clip: true

        delegate: Rectangle {
            width: 300; height: 380
            color: "#1a1a1a"; radius: 12
            border.color: Qt.rgba(1, 1, 1, 0.1)

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 15

                RowLayout {
                    Layout.fillWidth: true
                    Rectangle {
                        width: 32; height: 32; radius: 16; color: "#2ecc71"
                        Text { anchors.centerIn: parent; text: model.tableNumber; font.bold: true }
                    }
                    Text { text: "Order #" + model.orderId.substring(0,6); color: "white"; font.bold: true; Layout.fillWidth: true }
                    Text { text: model.time; color: "#95a5a6"; font.pixelSize: 12 }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(1,1,1,0.1); Layout.topMargin: 5; Layout.bottomMargin: 5 }

                ScrollView {
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    Text {
                        width: parent.width; text: model.items; color: "#ecf0f1"
                        font.pixelSize: 16; lineHeight: 1.4
                    }
                }

                Button {
                    Layout.fillWidth: true; Layout.preferredHeight: 45
                    text: "MARK SERVED"
                    onClicked: _salesModel.updateItemStatus(model.orderId, "served")

                    background: Rectangle {
                        color: "transparent"; border.color: "#2ecc71"; border.width: 1; radius: 6
                    }
                    contentItem: Text {
                        text: parent.text; color: "#2ecc71"; font.bold: true
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
}
