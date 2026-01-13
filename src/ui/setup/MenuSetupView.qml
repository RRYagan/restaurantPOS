import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: menuSetupRoot
    property var controller
    property var categoryModel
    property var taxModel
    property var unitModel

    property bool isEditing: false
    property var currentProduct: null

    // Background for the entire page
    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"
    }

    // Helper to find index for ComboBoxes
    function findIndexByValue(model, value, key) {
        if (!model) return 0;
        for (let i = 0; i < model.count; i++) {
            if (model.get(i)[key] === value) return i;
        }
        return 0;
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: menuSetupRoot.isEditing ? 1 : 0

        // --- VIEW 0: SEARCHABLE LIST ---
        ColumnLayout {
            spacing: 0

            // Header/Search Bar Area
            Rectangle {
                Layout.fillWidth: true
                height: 80
                color: "white"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20

                    TextField {
                        id: searchBar
                        placeholderText: "Search products..."
                        Layout.fillWidth: true
                        background: Rectangle {
                            radius: 10
                            color: "#F5F5F5"
                            border.color: "#E0E0E0"
                        }
                    }

                    Button {
                        text: "Add Product"
                        highlighted: true
                        onClicked: {
                            menuSetupRoot.currentProduct = null;
                            menuSetupRoot.isEditing = true;
                        }
                    }
                }
            }

            // Product List
            ListView {
                id: productList
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: controller.productModel
                clip: true
                spacing: 5

                delegate: ItemDelegate {
                    width: productList.width - 40
                    x: 20
                    height: 70

                    background: Rectangle {
                        color: hovered ? "#F8F9FA" : "white"
                        radius: 8
                        border.color: "#EEEEEE"
                        border.width: 1
                    }

                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: model.name
                                font.bold: true
                                font.pixelSize: 14
                                color: "#212121"
                            }
                            Text {
                                text: "Category: " + model.category
                                color: "#757575"
                                font.pixelSize: 11
                            }
                        }
                        Text {
                            text: "$" + parseFloat(model.price).toFixed(2)
                            color: "#2E7D32" // Themed Accent
                            font.bold: true
                            font.pixelSize: 15
                        }
                    }

                    onClicked: {
                        controller.productId = model.id
                        menuSetupRoot.currentProduct = {
                            "id": model.id,
                            "localId": model.localId,
                            "name": model.name,
                            "price": model.price,
                            "kraCode": model.kraCode,
                            "categoryCode": model.category,
                            "taxId": model.taxId,
                            "unitId": model.unitId
                        }
                        menuSetupRoot.isEditing = true
                    }
                }
            }
        }

        // --- VIEW 1: MASTER EDITOR ---
        ScrollView {
            contentWidth: availableWidth
            clip: true

            ColumnLayout {
                width: parent.width - 40
                x: 20
                spacing: 25

                // Editor Toolbar
                RowLayout {
                    Layout.topMargin: 20
                    Button {
                        text: "← Back"
                        flat: true
                        onClicked: menuSetupRoot.isEditing = false
                    }
                    Text {
                        text: currentProduct ? "Edit Product: " + currentProduct.name : "New Product"
                        font.pixelSize: 22
                        font.bold: true
                        color: "#212121"
                    }
                }

                // --- SECTION 1: MAIN PRODUCT DETAILS ---
                Rectangle {
                    Layout.fillWidth: true
                    height: editGrid.implicitHeight + 60
                    color: "white"
                    radius: 12
                    border.color: "#E0E0E0"

                    GridLayout {
                        id: editGrid
                        anchors.fill: parent
                        anchors.margins: 30
                        columns: 2
                        rowSpacing: 20
                        columnSpacing: 20

                        Label { text: "Display Name:"; font.bold: true }
                        TextField {
                            id: nameIn
                            text: currentProduct ? currentProduct.name : ""
                            Layout.fillWidth: true
                        }

                        Label { text: "Price ($):"; font.bold: true }
                        TextField {
                            id: priceIn
                            text: currentProduct ? currentProduct.price : ""
                            Layout.fillWidth: true
                        }

                        Label { text: "KRA Code:"; font.bold: true }
                        TextField {
                            id: kraIn
                            text: currentProduct ? currentProduct.kraCode : ""
                            Layout.fillWidth: true
                        }

                        Label { text: "Category:"; font.bold: true }
                        ComboBox {
                            id: catCombo
                            Layout.fillWidth: true
                            model: menuSetupRoot.categoryModel
                            textRole: "text"
                            valueRole: "code"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.categoryCode, "code") : 0
                        }

                        Label { text: "Tax Type:"; font.bold: true }
                        ComboBox {
                            id: taxCombo
                            Layout.fillWidth: true
                            model: menuSetupRoot.taxModel
                            textRole: "text"
                            valueRole: "valueId"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.taxId, "valueId") : 0
                        }

                        Label { text: "Unit:"; font.bold: true }
                        ComboBox {
                            id: unitCombo
                            Layout.fillWidth: true
                            model: menuSetupRoot.unitModel
                            textRole: "text"
                            valueRole: "valueId"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.unitId, "valueId") : 0
                        }

                        Button {
                            text: "Save Product Information"
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            height: 45
                            highlighted: true
                            enabled: nameIn.text.trim().length > 0
                            onClicked: {
                                let payload = {
                                    "id": currentProduct ? currentProduct.id : "",
                                    "name": nameIn.text,
                                    "price": parseFloat(priceIn.text) || 0.0,
                                    "kraCode": kraIn.text,
                                    "categoryCode": catCombo.currentValue,
                                    "taxId": parseInt(taxCombo.currentValue),
                                    "unitId": parseInt(unitCombo.currentValue)
                                }
                                if (controller.saveProduct(payload)) {
                                    menuSetupRoot.isEditing = false
                                } else {
                                    saveErrorAnim.start()
                                }
                            }
                        }
                    }
                }

                // --- SECTION 2: RECIPE / COMPOSITION ---
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 15
                    visible: currentProduct !== null && currentProduct.id !== ""

                    Text {
                        text: "Product Recipe / Ingredients"
                        font.bold: true
                        font.pixelSize: 18
                        color: "#212121"
                    }

                    // A. ADD INGREDIENT FORM
                    Rectangle {
                        Layout.fillWidth: true
                        height: 90
                        color: "#FDFDFD"
                        border.color: "#E0E0E0"
                        radius: 10

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 10

                            ColumnLayout {
                                Layout.fillWidth: true
                                Label { text: "Select Item"; font.pixelSize: 11; color: "#666" }
                                ComboBox {
                                    id: ingSelector
                                    Layout.fillWidth: true
                                    model: controller.productModel
                                    textRole: "name"
                                    valueRole: "id"
                                    currentIndex: -1
                                    displayText: currentIndex === -1 ? "Choose ingredient..." : currentText
                                }
                            }

                            ColumnLayout {
                                width: 80
                                Label { text: "Qty"; font.pixelSize: 11; color: "#666" }
                                TextField { id: ingQty; placeholderText: "0.0"; Layout.fillWidth: true }
                            }

                            ColumnLayout {
                                width: 120
                                Label { text: "Unit"; font.pixelSize: 11; color: "#666" }
                                ComboBox {
                                    id: ingUnitCombo
                                    Layout.fillWidth: true
                                    model: menuSetupRoot.unitModel
                                    textRole: "text"
                                    valueRole: "valueId"
                                    currentIndex: -1
                                }
                            }

                            Button {
                                text: "Add"
                                highlighted: true
                                Layout.alignment: Qt.AlignBottom
                                enabled: ingSelector.currentIndex >= 0 && ingQty.text.length > 0
                                onClicked: {
                                    controller.saveIngredient({
                                        "productId": currentProduct.id,
                                        "ingredientProductId": ingSelector.currentValue,
                                        "quantity": parseFloat(ingQty.text) || 1,
                                        "measurementUnitId": parseInt(ingUnitCombo.currentValue)
                                    })
                                    ingQty.text = "";
                                    ingSelector.currentIndex = -1;
                                    ingUnitCombo.currentIndex = -1;
                                }
                            }
                        }
                    }

                    // B. INGREDIENTS LIST (REPEATER)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            model: controller.compositionModel
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 55
                                color: "white"
                                radius: 8
                                border.color: "#EEEEEE"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 15

                                    // Quantity Badge
                                    Rectangle {
                                        width: 50; height: 28; color: "#E3F2FD"; radius: 6
                                        Text {
                                            anchors.centerIn: parent
                                            text: model.quantity
                                            color: "#1976D2"; font.bold: true
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            text: model.ingredientName || "Unknown Ingredient"
                                            font.bold: true; font.pixelSize: 14
                                        }
                                        Text {
                                            text: "ID: " + model.ingredientId
                                            font.pixelSize: 11; color: "#9E9E9E"
                                        }
                                    }

                                    Button {
                                        text: "Remove"
                                        flat: true
                                        contentItem: Text {
                                            text: "Remove"
                                            color: "#D32F2F"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        onClicked: controller.compositionModel.removeIngredient(model.id)
                                    }
                                }
                            }
                        }

                        // Placeholder if empty
                        Text {
                            text: "No ingredients added to this recipe yet."
                            visible: controller.compositionModel.count === 0
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 20
                            color: "#999"
                            font.italic: true
                        }
                    }
                }

                Item { height: 40; Layout.fillWidth: true } // Bottom spacer
            }
        }
    }

    SequentialAnimation on x {
        id: saveErrorAnim
        running: false
        NumberAnimation { to: 15; duration: 50 }
        NumberAnimation { to: 25; duration: 50 }
        NumberAnimation { to: 20; duration: 50 }
    }
}
