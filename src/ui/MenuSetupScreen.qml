import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: menuSetupRoot
    color: "transparent"
    property string selectedCategory: "All"
    readonly property bool isManager: globalUserModel.isAdmin

    CategoryView { id: catModel }
    MenuView { id: itemModel; currentCategory: menuSetupRoot.selectedCategory }

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
                    id: catList; Layout.fillHeight: true; Layout.fillWidth: true; model: catModel
                    delegate: ItemDelegate {
                        width: parent.width
                        highlighted: model.name === selectedCategory
                        onClicked: selectedCategory = model.name

                        contentItem: RowLayout {
                            Text { text: model.name; color: "white"; Layout.fillWidth: true }
                            Button {
                                text: "🗑"; flat: true; visible: model.name !== "All"
                                onClicked: {
                                    deleteDialog.title = "Delete Category"
                                    deleteDialog.itemLabel = model.name;
                                    deleteDialog.targetId = model.name;
                                    deleteDialog.targetModel = catModel;
                                    deleteDialog.deleteMethod = "deleteCategory";
                                    deleteDialog.onConfirmed = function() {
                                                itemModel.deleteCategory(model.id)
                                                console.log(model.name + " was removed.")
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
                Layout.fillWidth: true; Layout.fillHeight: true
                model: itemModel; cellWidth: 220; cellHeight: 180
                delegate: Rectangle {
                    width: 200; height: 160; color: Qt.rgba(1,1,1,0.1); radius: 10
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 10
                        Text { text: model.name; color: "white"; font.bold: true }
                        Text { text: (model.base_price_cents/100).toFixed(2) + " Ksh"; color: "#2ecc71" }
                        Button {
                            text: "Delete"
                            onClicked: {
                                deleteDialog.title = "Delete Menu Item"
                                deleteDialog.itemLabel = model.name;
                                deleteDialog.targetId = model.id;
                                deleteDialog.targetModel = itemModel;
                                deleteDialog.deleteMethod = "deleteItem";
                                deleteDialog.onConfirmed = function() {
                                            itemModel.deleteMenuItem(model.id)
                                            console.log(model.name + " was removed.")
                                        }
                                deleteDialog.open();
                            }
                        }
                    }
                }
            }
        }
    }
}
