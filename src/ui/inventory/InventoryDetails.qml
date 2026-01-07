import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: detailsPage
    color: "#1a0202" // Matching dark red theme

    property var itemData: null
    property var invView: null // The InventoryView instance

    Component.onCompleted: {
            // Set the proxy filterId to match the selected item
            invView.historyProxy.filterId = itemData.id
        }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 25

        // Header Section
        RowLayout {
            spacing: 20
            Button {
                text: "←"
                flat: true
                font.pixelSize: 24
                palette.buttonText: "white"
                onClicked: stackView.pop()
            }
            ColumnLayout {
                spacing: 2
                Text {
                    text: itemData ? itemData.name : "Unknown Item"
                    color: "white"
                    font.pixelSize: 24
                    font.bold: true
                }
                Text {
                    text: "Item ID: #" + (itemData ? itemData.id : "0")
                    color: "#95a5a6"
                    font.pixelSize: 14
                }
            }
        }

        // Status Overview Cards
        RowLayout {
            spacing: 20
            Layout.fillWidth: true

            Rectangle {
                Layout.fillWidth: true
                height: 100
                color: Qt.rgba(1, 1, 1, 0.05)
                radius: 12
                Column {
                    anchors.centerIn: parent
                    spacing: 5
                    Text { text: "CURRENT STOCK"; color: "#95a5a6"; font.pixelSize: 12; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    Text {
                        text: itemData ? itemData.quantity + " " + (itemData.unit || "pcs") : "0"
                        color: "#2ecc71"
                        font.pixelSize: 32; font.bold: true
                    }
                }
            }
        }

        Text {
            text: "Movement History"
            color: "white"
            font.pixelSize: 18
            font.bold: true
            Layout.topMargin: 10
        }

        // History Table Header
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: Qt.rgba(1, 1, 1, 0.08)
            radius: 6
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15; anchors.rightMargin: 15
                Text { text: "DATE & TIME"; color: "#95a5a6"; font.bold: true; Layout.preferredWidth: 180 }
                Text { text: "ACTION"; color: "#95a5a6"; font.bold: true; Layout.preferredWidth: 100 }
                Text { text: "DETAILS"; color: "#95a5a6"; font.bold: true; Layout.fillWidth: true }
            }
        }

        // History List
        ListView {
            id: historyList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: invView.historyProxy
            spacing: 5

            Binding {
                    target: invView.historyProxy
                    property: "filterId"
                    value: itemData ? itemData.id : -1
                }

            delegate: Rectangle {
                width: historyList.width
                height: 50
                color: index % 2 === 0 ? "transparent" : Qt.rgba(1, 1, 1, 0.02)
                radius: 4

                RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15

                            Text {
                                text: model.displayData.timestamp
                                color: "#95a5a6"
                                Layout.preferredWidth: 150
                            }
                            Text {
                                text: model.displayData.action
                                color: model.displayData.action === "ADD" ? "#3498db" : "#f1c40f"
                                font.bold: true
                                Layout.preferredWidth: 100
                            }
                            Text {
                                text: model.displayData.details
                                color: "white"
                                Layout.fillWidth: true
                            }
                        }}
        }
    }
}
