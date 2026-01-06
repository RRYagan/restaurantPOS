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

    Loader {
        id: confirmLoader
        active: false
        anchors.fill: parent
        sourceComponent: ConfirmDialog {
            id: invDeleteDialog
            // The dialog will automatically close and 'active' stays true
            // until we manually reset it or change pages
        }
    }

    // Loader for the Add/Edit Stock Dialog
    Loader {
        id: stockLoader
        anchors.fill: parent
        active: false
        sourceComponent: StockDialog { id: internalStockDialog }
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
                onTextChanged: invModel.searchString = text

                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    radius: 8
                    border.color: searchBar.activeFocus ? "#c0392b" : Qt.rgba(1, 1, 1, 0.2)
                }
            }
            // Near your Search Bar RowLayout
            ComboBox {
                id: filterStatus
                model: ["All Items", "Low Stock", "Out of Stock"]
                Layout.preferredHeight: 45
                Layout.preferredWidth: 150
                onCurrentIndexChanged: {
                        // Assuming invModel is your InventoryView which exposes the proxy
                        invModel.filterMode = currentIndex
                    }
            }
            Button {
                id: addBtn
                text: "+ Add Stock"
                visible: inventoryPage.isManager
                // Match the height of the TextField
                Layout.preferredHeight: 45
                Layout.preferredWidth: 140

                contentItem: Text {
                    text: addBtn.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }

                background: Rectangle {
                    color: addBtn.pressed ? "#a03023" : (addBtn.hovered ? "#d35400" : "#c0392b")
                    radius: 8
                }

                onClicked: {
                    stockLoader.active = true
                    stockLoader.item.openForAdd()
                }
            }
        }


        Loader {
            id: menuLoader
            active: false
            visible: false
            source: "InventoryContextMenu.qml"

            Connections {
                target: menuLoader.item
                ignoreUnknownSignals: true

                // --- ACTION: DELETE ---
                function onDeleteRequested(data) {
                    confirmLoader.active = true
                    var invDeleteDialog = confirmLoader.item

                    invDeleteDialog.title = "Delete Stock Item"
                    invDeleteDialog.itemLabel = data.name
                    // Use your existing property-based logic
                    invDeleteDialog.targetModel = globalInventoryModel
                    invDeleteDialog.targetId = data.id
                    invDeleteDialog.deleteMethod = "deleteStock"
                    invDeleteDialog.onConfirmed = null // Use the built-in method call
                    invDeleteDialog.open()
                }

                // --- ACTION: QUICK UPDATE (+1 Stock) ---
                function onUpdateRequested(data) {
                    confirmLoader.active = true
                    var invDeleteDialog = confirmLoader.item

                    invDeleteDialog.title = "Confirm Stock Update"
                    invDeleteDialog.itemLabel = "+1 to " + data.name

                    // Use a callback for the specific update logic
                    invDeleteDialog.onConfirmed = function() {
                        globalInventoryModel.updateStock(
                            data.id,
                            data.name,
                            data.quantity + 1,
                            data.unit
                        )
                    }
                    invDeleteDialog.open()
                }

                // --- ACTION: EDIT ---
                function onEditRequested(data) {
                    // console.log("data on edit", data)
                    if (data) {
                        stockLoader.active = true

                        stockLoader.item.openForEdit(data)
                }
                }
            }
        }// Inventory List
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
                            text: model.displayData.name || ""
                            color: "white"
                            font.bold: true
                            font.pixelSize: 16
                        }
                        Text {
                            text: "SKU: " + (model.displayData.sku || "N/A")
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
                            color: model.displayData.quantity < 10 ? Qt.rgba(1, 0, 0, 0.2) : Qt.rgba(0, 1, 0, 0.1)
                            Text {
                                anchors.centerIn: parent
                                text: model.displayData.quantity + " " + (model.displayData.unit || "pcs")
                                color: model.displayData.quantity < 10 ? "#ff7675" : "#2ecc71"
                                font.pixelSize: 12; font.bold: true
                            }
                        }

                        // The Edit ":" Button
                        Button {
                            id: moreButton
                            text: "⋮" // Vertical ellipsis for "More/Edit"
                            flat: true
                            font.pixelSize: 20
                            palette.buttonText: "white"

                            onClicked: {
                                menuLoader.active = true
                                menuLoader.item.targetData = model.displayData
                                menuLoader.item.popup(moreButton, 0, moreButton.height)
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
