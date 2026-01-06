import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: inventoryPage
    color: "transparent" // Reveal dark red background from Main.qml

    property var invModel
    property var usrModel
    readonly property bool isManager: usrModel ? usrModel.isAdmin : false

    // Unified Delete Dialog instance
    ConfirmDeleteDialog {
        id: invDeleteDialog
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        Text {
            text: "Stock & Inventory"
            color: "white"
            font.pixelSize: 24
            font.bold: true
        }

        // Search and Filter Header
        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: searchBar
                placeholderText: "Search inventory..."
                Layout.fillWidth: true
                color: "white"
                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    radius: 8
                    border.color: Qt.rgba(1, 1, 1, 0.2)
                }
            }
            Button {
                text: "+ Add Stock"
                visible: inventoryPage.isManager
                palette.button: "#c0392b"
                palette.buttonText: "white"
                // onClicked: addStockDialog.open()
            }
        }

        // Inventory List
        ListView {
            id: invList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: invModel
            clip: true
            spacing: 12

            delegate: Rectangle {
                width: invList.width
                height: 70
                color: Qt.rgba(0.15, 0.02, 0.02, 0.8) // Dark glassy red
                radius: 10
                border.color: Qt.rgba(1, 1, 1, 0.1)

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    // 1. Left Side: Item Info
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: model.name
                            color: "white"
                            font.bold: true
                            font.pixelSize: 16
                        }
                        Text {
                            text: "SKU: " + (model.sku || "N/A")
                            color: "#95a5a6"
                            font.pixelSize: 12
                        }
                    }

                    // 2. Middle: Spacer (This pushes everything after it to the right)
                    Item {
                        Layout.fillWidth: true
                    }

                    // 3. Right Side: Stock Status & Actions
                    RowLayout {
                        spacing: 20

                        // Stock Badge
                        Rectangle {
                            width: 80; height: 26; radius: 13
                            color: model.quantity < 10 ? Qt.rgba(1, 0, 0, 0.2) : Qt.rgba(0, 1, 0, 0.1)
                            Text {
                                anchors.centerIn: parent
                                text: model.quantity + " " + (model.unit || "pcs")
                                color: model.quantity < 10 ? "#ff7675" : "#2ecc71"
                                font.pixelSize: 12; font.bold: true
                            }
                        }

                        // The Edit ":" Button
                        Button {
                            text: "⋮" // Vertical ellipsis for "More/Edit"
                            flat: true
                            font.pixelSize: 20
                            palette.buttonText: "white"

                            onClicked: {
                                // Logic to open your edit dialog
                                console.log("Editing item:", model.name)
                            }

                            background: Rectangle {
                                color: parent.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                                radius: 4
                            }
                        }
                    }
                }
            }
        }
    }
}
