import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales

Rectangle {
    id: setupRoot
    color: "#f4f7f6"

    // selectedCategory acts as the "bridge" between the two models
    property string selectedCategory: "All"

    // Use the registered types directly
    CategoryModel { id: catModel }
    MenuModel {
        id: itemModel
        currentCategory: setupRoot.selectedCategory
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Sidebar: Category Management ---
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: "white"
            border.color: "#e0e0e0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12

                Text { text: "Manage Categories"; font.bold: true; font.pixelSize: 18 }

                // Add Category Row
                RowLayout {
                    TextField {
                        id: catIn; placeholderText: "New..."; Layout.fillWidth: true
                    }
                    Button {
                        text: "+"; highlighted: true
                        onClicked: {
                            if(catModel.addCategory(catIn.text)) catIn.clear()
                        }
                    }
                }

                ListView {
                    id: catList
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: catModel
                    clip: true
                    spacing: 2
                    delegate: ItemDelegate {
                        width: catList.width
                        highlighted: setupRoot.selectedCategory === model.name
                        onClicked: setupRoot.selectedCategory = model.name

                        contentItem: RowLayout {
                            Text {
                                text: model.name
                                Layout.fillWidth: true
                                font.bold: highlighted
                            }
                            Button {
                                text: "×"; visible: model.name !== "All"
                                flat: true; palette.buttonText: "red"
                                onClicked: catModel.deleteCategory(model.name)
                            }
                        }
                    }
                }
            }
        }

        // --- Main Content: Item Management ---
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 20

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: selectedCategory + " Items"
                        font.pixelSize: 22; font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: "Add New Item"; highlighted: true
                        enabled: selectedCategory !== "All"
                        onClicked: addItemDialog.open()
                    }
                }

                GridView {
                    id: itemGrid
                    Layout.fillWidth: true; Layout.fillHeight: true
                    clip: true; cellWidth: 240; cellHeight: 140
                    model: itemModel

                    delegate: Rectangle {
                        width: 220; height: 120
                        color: "white"; radius: 6; border.color: "#dcdde1"

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 12
                            Text { text: model.name; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
                            Text {
                                text: (model.base_price_cents / 100).toFixed(2) + " Ksh."
                                color: "#27ae60"; font.bold: true
                            }
                            Item { Layout.fillHeight: true }
                            Button {
                                text: "Delete"; palette.buttonText: "red"
                                Layout.alignment: Qt.AlignRight
                                onClicked: itemModel.deleteItem(model.id)
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: addItemDialog
        title: "Add to " + selectedCategory; modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Save | Dialog.Cancel

        ColumnLayout {
            TextField { id: nameIn; placeholderText: "Item Name"; Layout.fillWidth: true }
            TextField { id: priceIn; placeholderText: "Price (Cents)"; Layout.fillWidth: true }
        }

        onAccepted: {
            if (itemModel.addMenuItem(nameIn.text, selectedCategory, parseInt(priceIn.text), "qrc:/assets/icons/default.svg")) {
                nameIn.clear(); priceIn.clear();
            }
        }
    }
}
