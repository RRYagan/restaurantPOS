import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: menuSetupRoot
    color: "#f4f7f6"

    property var itemDeleteDialog
    property var catDeleteDialog
    // selectedCategory acts as the "bridge" between the two models
    property string selectedCategory: "All"
    readonly property bool isManager: globalUserModel.isAdmin

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
                        color: "#000000"
                        // visible:menuSetupRoot.isManager && selectedCategory !== "All"
                        background: Rectangle {
                                implicitWidth: 200
                                implicitHeight: 40

                                border.color: (catIn.text.trim() === "" && catIn.focus) ? "#e74c3c" : "#e0e0e0"
                                border.width: 1
                            }
                    }

                    Button {
                        text: "+";
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
                    id: catList
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: catModel
                    clip: true
                    spacing: 2
                    delegate: ItemDelegate {
                        width: catList.width
                        highlighted: menuSetupRoot.selectedCategory === model.name
                        onClicked: menuSetupRoot.selectedCategory = model.name

                        contentItem: RowLayout {
                            Text {
                                text: model.name
                                Layout.fillWidth: true
                                font.bold: highlighted
                            }
                            Button {
                                    text: "🗑"
                                    visible: menuSetupRoot.isManager && model.name !=="All"
                                    onClicked: {
                                        if (menuSetupRoot.catDeleteDialog !== null) {
                                        menuSetupRoot.catDeleteDialog.catNameToDelete = model.name
                                        menuSetupRoot.catDeleteDialog.open()
                                        } else {
                                            console.log("Error: catDeleteDialog not linked!");

                                        }
                                    }
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
                        enabled: menuSetupRoot.isManager && selectedCategory !== "All"
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
                                    text: "Delete Item"
                                    visible: menuSetupRoot.isManager
                                    enabled: menuSetupRoot.isManager && selectedCategory !== "All"

                                    onClicked: {
                                        if (menuSetupRoot.itemDeleteDialog !== null) {
                                                        menuSetupRoot.itemDeleteDialog.itemIdToDelete = model.id;
                                                        menuSetupRoot.itemDeleteDialog.open();
                                                    } else {
                                                        console.log("Error: itemDeleteDialog not linked!");
                                                    }
                                    }
                                }
                        }
                    }
                }
            }
        }
    }
    Dialog {
        id: addItemDialog
        title: "Add to " + selectedCategory
        modal: true
        // width: 350
        anchors.centerIn: parent
        // Do NOT use standardButtons: Dialog.Save here

        ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10

            TextField {
                id: nameIn
                placeholderText: "Item Name"
                color: "black" // Font color black
                Layout.fillWidth: true
                background: Rectangle {
                    border.color: (validationError.visible && nameIn.text.trim() === "") ? "#e74c3c" : "#bdc3c7"
                    border.width: 1
                }
            }

            TextField {
                id: priceIn
                placeholderText: "Price (Cents)"
                color: "black" // Font color black
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                background: Rectangle {
                    border.color: (validationError.visible && (priceIn.text.trim() === "" || isNaN(parseInt(priceIn.text)))) ? "#e74c3c" : "#bdc3c7"
                    border.width: 1
                }
            }

            Text {
                            id: validationError
                            text: "Please enter a valid name and price."
                            color: "#e74c3c"
                            visible: false
                            font.pixelSize: 12
                            font.italic: true
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap // Ensures error text doesn't push dialog wider
                        }
        }

        footer: DialogButtonBox {
            // Use a standard Button but avoid the automatic "AcceptRole" if it still closes
            Button {
                text: "Save"
                onClicked: {
                    var name = nameIn.text.trim();
                    var priceStr = priceIn.text.trim();
                    var price = parseInt(priceStr);

                    // Validation: Check for null/empty and positive number
                    if (name !== "" && priceStr !== "" && !isNaN(price) && price > 0) {
                        if (itemModel.addMenuItem(name, selectedCategory, price, "qrc:/assets/icons/default.svg")) {
                            nameIn.clear();
                            priceIn.clear();
                            validationError.visible = false;
                            addItemDialog.close(); // Manual close only on success
                        }
                    } else {
                        validationError.visible = true; // Show message, dialog stays open
                    }
                }
            }
            Button {
                text: "Cancel"
                onClicked: {
                    nameIn.clear();
                    priceIn.clear();
                    validationError.visible = false;
                    addItemDialog.close();
                }
            }
        }
    }

}
