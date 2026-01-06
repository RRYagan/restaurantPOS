import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: menuSetupRoot
    color: "transparent" // Reveal background from Main.qml

    property var itemDeleteDialog
    property var catDeleteDialog
    property string selectedCategory: "All"
    readonly property bool isManager: globalUserModel.isAdmin

    CategoryView { id: catModel }
    MenuView { id: itemModel }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Sidebar: Category Management (Glassy Style) ---
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4) // Dark translucent glass
            border.color: Qt.rgba(255, 255, 255, 0.1)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12

                Text {
                    text: "Manage Categories"
                    font.bold: true
                    font.pixelSize: 18
                    color: "white"
                }

                // Add Category Row
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
                    id: catList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: catModel
                    clip: true
                    spacing: 4
                    delegate: ItemDelegate {
                        width: catList.width
                        highlighted: menuSetupRoot.selectedCategory === model.name
                        onClicked: menuSetupRoot.selectedCategory = model.name

                        background: Rectangle {
                            color: highlighted ? Qt.rgba(1, 1, 1, 0.15) : (hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
                            radius: 4
                        }

                        contentItem: RowLayout {
                            Text {
                                text: model.name
                                Layout.fillWidth: true
                                color: "white"
                                font.bold: highlighted
                            }
                            Button {
                                text: "🗑"
                                flat: true
                                visible: menuSetupRoot.isManager && model.name !=="All"
                                palette.buttonText: "#ff7675"
                                onClicked: {
                                    if (menuSetupRoot.catDeleteDialog !== null) {
                                        console.log("model.name", model.name)
                                        menuSetupRoot.catDeleteDialog.catNameToDelete = model.name
                                        menuSetupRoot.catDeleteDialog.open()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- Main Content: Item Management (Glassy Grid) ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

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
                    id: itemGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    cellWidth: 240
                    cellHeight: 140
                    model: itemModel

                    delegate: Rectangle {
                        width: 220
                        height: 120
                        color: Qt.rgba(1, 1, 1, 0.15) // Frosted glass item card
                        radius: 8
                        border.color: Qt.rgba(255, 255, 255, 0.1)

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12

                            Text {
                                text: model.name
                                font.bold: true
                                color: "white"
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Text {
                                text: (model.base_price_cents / 100).toFixed(2) + " Ksh."
                                color: "#2ecc71"
                                font.bold: true
                            }

                            Item { Layout.fillHeight: true }

                            Button {
                                text: "Delete Item"
                                Layout.fillWidth: true
                                palette.buttonText: "#ff7675"
                                visible: menuSetupRoot.isManager
                                onClicked: {
                                    if (menuSetupRoot.itemDeleteDialog !== null) {
                                        menuSetupRoot.itemDeleteDialog.itemIdToDelete = model.id;
                                        menuSetupRoot.itemDeleteDialog.open();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Add Item Dialog ---
    Dialog {
        id: addItemDialog
        title: "Add to " + selectedCategory
        modal: true
        anchors.centerIn: parent

        background: Rectangle {
            color: "#2c3e50"
            radius: 8
            border.color: "#34495e"
        }

        header: Label {
            text: addItemDialog.title
            color: "white"
            padding: 15
            font.bold: true
        }

        ColumnLayout {
            spacing: 15
            width: 300

            TextField {
                id: nameIn
                placeholderText: "Item Name"
                color: "white"
                Layout.fillWidth: true
                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    border.color: (validationError.visible && nameIn.text.trim() === "") ? "#e74c3c" : "#bdc3c7"
                }
            }

            TextField {
                id: priceIn
                placeholderText: "Price (Cents)"
                color: "white"
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                background: Rectangle {
                    color: Qt.rgba(1, 1, 1, 0.1)
                    border.color: (validationError.visible && (priceIn.text.trim() === "" || isNaN(parseInt(priceIn.text)))) ? "#e74c3c" : "#bdc3c7"
                }
            }

            Text {
                id: validationError
                text: "Please enter a valid name and price."
                color: "#e74c3c"
                visible: false
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
        }

        footer: DialogButtonBox {
            Button {
                text: "Save"
                onClicked: {
                    var name = nameIn.text.trim();
                    var priceStr = priceIn.text.trim();
                    var price = parseInt(priceStr);

                    if (name !== "" && priceStr !== "" && !isNaN(price) && price > 0) {
                        if (itemModel.addMenuItem(name, selectedCategory, price, "qrc:/assets/icons/default.svg")) {
                            nameIn.clear();
                            priceIn.clear();
                            validationError.visible = false;
                            addItemDialog.close();
                        }
                    } else {
                        validationError.visible = true;
                    }
                }
            }
            Button {
                text: "Cancel"
                onClicked: addItemDialog.close()
            }
        }
    }
}
