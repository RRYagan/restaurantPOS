import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: menuSetupRoot
    color: "transparent"
    property string selectedCategory: "All"
    onSelectedCategoryChanged: {
        if (itemModel.proxy) {
            itemModel.proxy.categoryFilter = selectedCategory;
        }
    }
    readonly property bool isManager: globalUserModel.isAdmin

    CategoryView { id: catModel }
    MenuView { id: itemModel }

    // Dialog Instances
    ConfirmDialog { id: deleteDialog }
    AddItemDialog {
        id: addItemDialog
        category: menuSetupRoot.selectedCategory
        itemModel: itemModel
    }

    RowLayout {
        anchors.fill: parent; spacing: 0

        // Sidebar
        Rectangle {
            Layout.preferredWidth: 280; Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4); border.color: Qt.rgba(255, 255, 255, 0.1)

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 15

                // Add Category Input
                RowLayout {
                    TextField {
                        id: catIn
                        placeholderText: "New..."
                        Layout.fillWidth: true
                        color: "white"
                        placeholderTextColor: "#8899aa"
                        background: Rectangle {
                            implicitHeight: 40
                            color: Qt.rgba(1, 1, 1, 0.1)
                            radius: 4
                            border.color: (catIn.text.trim() === "" && catIn.focus) ? "#e74c3c" : Qt.rgba(255, 255, 255, 0.2)
                        }
                    }

                    Button {
                        text: "+"
                        highlighted: true
                        visible: menuSetupRoot.isManager
                        onClicked: {
                            var input = catIn.text.trim()
                            if (input !== "") {
                                if(catModel.addCategory(input)) catIn.clear()
                            } else {
                                catErrorTip.show("Please enter a category name")
                            }
                        }
                        ToolTip {
                            id: catErrorTip
                            timeout: 2000
                            function show(msg) { text = msg; open(); }
                        }
                    }
                }

                ListView {
                    id: catList; Layout.fillHeight: true; Layout.fillWidth: true; model: catModel.proxy
                    delegate: ItemDelegate {
                        width: parent.width
                        highlighted: model.displayData.name === selectedCategory
                        onClicked: selectedCategory = model.displayData.name

                        contentItem: RowLayout {
                            Text { text: model.displayData.name; color: "white"; Layout.fillWidth: true }
                            Button {
                                text: "🗑"; flat: true; visible: model.name !== "All"
                                onClicked: {
                                    deleteDialog.title = "Delete Category"
                                    deleteDialog.itemLabel = model.displayData.name;
                                    deleteDialog.targetId = model.displayData.name;
                                    deleteDialog.targetModel = catModel;
                                    deleteDialog.deleteMethod = "deleteCategory";
                                    deleteDialog.onConfirmed = function() {

                                                catModel.deleteCategory(model.displayData.name)
                                                // console.log(model.name + " was removed.")
                                            }
                                    deleteDialog.open();
                                }
                            }
                        }
                    }
                }
            }
        }

        // Main Grid
        ColumnLayout {
            Layout.fillWidth: true; Layout.margins: 20
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: selectedCategory + " Items"
                    font.pixelSize: 22
                    font.bold: true
                    color: "white"
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: "Add New Item"
                    highlighted: true
                    enabled: menuSetupRoot.isManager && selectedCategory !== "All"
                    onClicked: addItemDialog.open()
                }
            }
            GridView {
                id: menuGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: itemModel.proxy
                cellWidth: 220
                cellHeight: 180
                clip: true

                delegate: Rectangle {
                    width: 200
                    height: 160
                    color: Qt.rgba(1, 1, 1, 0.1)
                    radius: 10
                    border.color: Qt.rgba(255, 255, 255, 0.05)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 5

                        // Top Row: Menu Options Button
                        RowLayout {
                            Layout.fillWidth: true
                            Item { Layout.fillWidth: true } // Spacer to push button to right

                            Button {
                                id: optionsButton
                                text: "⋮" // Vertical ellipsis
                                flat: true
                                font.pixelSize: 20
                                palette.buttonText: "white"
                                Layout.preferredWidth: 30
                                Layout.preferredHeight: 30

                                onClicked: optionsMenu.open()

                                Menu {
                                    id: optionsMenu
                                    y: optionsButton.height

                                    MenuItem {
                                        text: "Edit"
                                        onTriggered: {
                                            // Set the shared dialog to 'Edit' mode
                                            addItemDialog.itemData = model.displayData;
                                            addItemDialog.open();
                                        }
                                    }

                                    MenuItem {
                                        text: "Delete"
                                        onTriggered: {
                                            deleteDialog.title = "Delete Menu Item"
                                            deleteDialog.itemLabel = model.displayData.name;
                                            deleteDialog.onConfirmed = function() {
                                                itemModel.deleteItem(model.displayData.id)
                                            }
                                            deleteDialog.open();
                                        }
                                    }
                                }
                            }
                        }

                        // Middle: Item Details
                        Text {
                            text: model.displayData.name
                            color: "white"
                            font.bold: true
                            font.pixelSize: 16
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        Text {
                            text: (model.displayData.price_cents / 100).toFixed(2) + " Ksh"
                            color: "#2ecc71"
                            font.pixelSize: 14
                        }

                        Item { Layout.fillHeight: true } // Spacer
                    }
                }
            }
        }
    }
}
