import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: inventoryPage
    color: "transparent"

    // invModel here should be your InventoryViewController instance
    property var invModel
    property var usrModel
    readonly property bool isManager: usrModel ? usrModel.isAdmin : false

    Loader {
        id: confirmLoader
        active: false
        anchors.fill: parent
        sourceComponent: ConfirmDialog { id: invDeleteDialog }
    }

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

        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            TextField {
                id: searchBar
                placeholderText: "Search inventory..."
                Layout.fillWidth: true
                Layout.preferredHeight: 45
                color: "white"
                verticalAlignment: TextInput.AlignVCenter
                leftPadding: 15
                // Search logic (Assuming your Proxy is exposed via the controller)
                onTextChanged: invModel.inventoryModel.searchString = text

                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    radius: 8
                    border.color: searchBar.activeFocus ? "#c0392b" : Qt.rgba(1, 1, 1, 0.2)
                }
            }

            ComboBox {
                id: filterStatus
                model: ["All Items", "Low Stock", "Out of Stock"]
                Layout.preferredHeight: 45
                Layout.preferredWidth: 150
                onCurrentIndexChanged: invModel.inventoryModel.filterMode = currentIndex
            }

            Button {
                id: addBtn
                text: "+ Add Stock"
                Layout.preferredHeight: 45
                Layout.preferredWidth: 140
                onClicked: {
                    stockLoader.active = true
                    stockLoader.item.openForAdd()
                }
                background: Rectangle {
                    color: addBtn.pressed ? "#a03023" : (addBtn.hovered ? "#d35400" : "#c0392b")
                    radius: 8
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

                // --- ACTION: DELETE (Updated for new deleteStock(QString)) ---
                function onDeleteRequested(data) {
                    confirmLoader.active = true
                    var dialog = confirmLoader.item
                    dialog.title = "Delete Stock Item"
                    dialog.itemLabel = data.name

                    // Direct call to controller via confirm callback
                    dialog.onConfirmed = function() {
                        invModel.deleteStock(data.id)
                    }
                    dialog.open()
                }

                // --- ACTION: QUICK UPDATE (Updated for updateStock(QVariantMap)) ---
                function onUpdateRequested(data) {
                    confirmLoader.active = true
                    var dialog = confirmLoader.item
                    dialog.title = "Confirm Stock Update"
                    dialog.itemLabel = "+1 to " + data.name

                    dialog.onConfirmed = function() {
                        // Pass a Map/Object to the controller
                        invModel.updateStock({
                            "id": data.id,
                            "name": data.name,
                            "quantityAvailable": data.quantityAvailable + 1,
                            "quantityUnitId": data.quantityUnitId
                        })
                    }
                    dialog.open()
                }

                function onEditRequested(data) {
                    if (data) {
                        stockLoader.active = true
                        stockLoader.item.openForEdit(data)
                    }
                }
            }
        }

        ListView {
            id: invList
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Use the model property from the controller
            model: invModel.inventoryModel
            clip: true
            spacing: 12

            delegate: Rectangle {
                width: invList.width
                height: 70
                color: Qt.rgba(0.15, 0.02, 0.02, 0.8)
                radius: 10
                border.width: model.id === invModel.inventoryId ? 2 : 0
                border.color: "#c0392b"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15

                    MouseArea {
                        id: infoMouseArea
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        hoverEnabled: true
                        onClicked: {
                            // Update selection in controller
                            invModel.inventoryId = model.id
                            contentStack.push("InventoryDetails.qml", {
                                "itemData": model,
                                "invController": invModel
                            })
                        }

                        RowLayout {
                            anchors.fill: parent
                            spacing: 15

                            ColumnLayout {
                                Layout.preferredWidth: 200
                                Text { text: model.name; color: "white"; font.bold: true }
                                Text { text: "ID: #" + model.id; color: "#95a5a6"; font.pixelSize: 11 }
                            }

                            Item { Layout.fillWidth: true }

                            Rectangle {

                                // return {
                                //     { IdRole, "id" },
                                //     { NameRole, "name" },
                                //     { QuantityAvailableRole, "quantityAvailable" },
                                //     { QuantityUnitNameRole, "quantityUnitName" }, // New
                                //     { PackagesAvailableRole, "packagesAvailable" },
                                //     { PackagingUnitNameRole, "packagingUnitName" }, // New
                                //     {QuantityPerPackageRole, "quantityPerPackage"},
                                //     { CreatedAtRole, "createdAt" },
                                //     { UpdatedAtRole, "updatedAt" }
                                // };
                                width: 110; height: 26; radius: 13
                                color: model.packagesAvailable < 10 ? Qt.rgba(1, 0, 0, 0.2) : Qt.rgba(0, 1, 0, 0.1)
                                RowLayout {
                                    anchors.centerIn: parent; spacing: 4
                                    Text {
                                        text: model.packagesAvailable
                                        color: model.packagesAvailable < 10 ? "#ff7675" : "#2ecc71"
                                        font.bold: true
                                    }
                                    Text {
                                        text: model.packagingUnitName
                                        color: model.packagesAvailable < 10 ? "#ff7675" : "#2ecc71"
                                        font.pixelSize: 10
                                    }
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Rectangle {

                                // int packagesAvailable;
                                // int packagingUnitId;
                                // double quantityPerPackage;
                                // double quantityAvailable;
                                // int quantityUnitId;
                                width: 110; height: 26; radius: 13
                                color: model.quantityAvailable < 10 ? Qt.rgba(1, 0, 0, 0.2) : Qt.rgba(0, 1, 0, 0.1)
                                RowLayout {
                                    anchors.centerIn: parent; spacing: 4
                                    Text {
                                        text: model.quantityAvailable
                                        color: model.quantityAvailable < 10 ? "#ff7675" : "#2ecc71"
                                        font.bold: true
                                    }
                                    Text {
                                        text: model.quantityUnitName
                                        color: model.quantityAvailable < 10 ? "#ff7675" : "#2ecc71"
                                        font.pixelSize: 10
                                    }
                                }
                            }
                        }
                    }

                    Button {
                        id: moreButton
                        text: "⋮"; flat: true
                        Layout.preferredWidth: 45
                        onClicked: {
                            invModel.inventoryId = model.id
                            menuLoader.active = true
                            menuLoader.item.targetData = model
                            var pos = moreButton.mapToItem(inventoryPage, 0, moreButton.height)
                            menuLoader.item.popup(pos.x - (menuLoader.item.width - moreButton.width), pos.y)
                        }
                    }
                }
            }
        }
    }
}
