import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: menuSetupRoot
    property var controller
    property var categoryModel
    property var taxModel
    property var unitModel
    property var inventoryModel
    property var typeModel

    property bool isEditing: false
    property var currentProduct: null

    // Background for the entire page
    Rectangle {
        anchors.fill: parent
        color: "transparent"
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
                // color: "white"
                color: "transparent"

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
                            // border.color: "#E0E0E0"
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
                    height: 75 // Slightly increased height for more data

                    background: Rectangle {
                        color: hovered ? "#F8F9FA" : "white"
                        radius: 8
                        border.color: highlighted ? "#2E7D32" : "#EEEEEE"
                        border.width: highlighted ? 2 : 1
                    }

                    contentItem: RowLayout {
                        spacing: 15

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                // Refactored Name: internalProductName
                                text: model.internalProductName
                                font.bold: true
                                font.pixelSize: 14
                                color: "#212121"
                            }

                            RowLayout {
                                spacing: 10
                                Text {
                                    // Refactored Category: productCategoryId
                                    text: "Cat: " + model.productCategoryId
                                    color: "#757575"
                                    font.pixelSize: 11
                                }
                                Text {
                                    // NEW: countryOriginId
                                    text: " | Origin: " + model.countryCode
                                    color: "#9E9E9E"
                                    font.pixelSize: 11
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.alignment: Qt.AlignRight
                            spacing: 0

                            Text {
                                // Refactored Price: defaultSellingPrice
                                // Showing currencyId dynamically
                                text: model.currencyCode+ " " + parseFloat(model.defaultSellingPrice).toFixed(2)
                                color: "#2E7D32"
                                font.bold: true
                                font.pixelSize: 15
                            }

                            Text {
                                // NEW: taxAmount
                                text: "+" + parseFloat(model.taxAmount).toFixed(2) + " Tax"
                                color: "#757575"
                                font.pixelSize: 10
                                visible: model.taxAmount > 0
                            }
                        }
                    }

                    onClicked: {
                        controller.productId = model.id
                        // Map the QVariantMap to exactly match the Controller's saveProduct expectations
                        menuSetupRoot.currentProduct = {
                            "id": model.id,
                            "internalProductName": model.internalProductName,
                            "productCategoryId": model.productCategoryId,
                            "kraItemCode": model.kraItemCode,
                            // "inventoryProductId": model.inventoryProductId,
                            "productTypeId": model.productTypeId,
                            "currencyCode": model.currencyCode, // Ensure this matches your model's role name
                            "countryCode": model.countryCode,
                            "defaultSellingPrice": model.defaultSellingPrice,
                            "taxAmount": model.taxAmount,
                            "taxClassificationCode": model.taxClassificationCode,

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
                        rowSpacing: 12
                        columnSpacing: 20

                        // 1. Identification
                        Label { text: "Display Name:"; font.bold: true }
                        TextField {
                            id: nameIn
                            text: currentProduct ? currentProduct.internalProductName : ""
                            Layout.fillWidth: true
                        }

                        Label { text: "KRA Code:"; font.bold: true }
                        TextField {
                            id: kraIn
                            text: currentProduct ? currentProduct.kraItemCode : ""
                            Layout.fillWidth: true
                        }
                        // 4. Financials
                        Label { text: "Price:"; font.bold: true }
                        TextField {
                            id: priceIn
                            text: currentProduct ? currentProduct.defaultSellingPrice : ""
                            Layout.fillWidth: true
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                        }
                        // 2. mandatory Region/Currency (NEW)
                        Label { text: "Currency:"; font.bold: true }
                        ComboBox {
                            id: currencyCombo
                            Layout.fillWidth: true
                            model: _currencyModel
                            textRole: "currency_code"
                            valueRole: "currency_code"
                            currentIndex: currentProduct ? indexOfValue(currentProduct.currencyCode) : indexOfValue("KES")
                        }


                        Label { text: "Country of Origin:"; font.bold: true }
                        ComboBox {
                            id: countryCombo
                            Layout.fillWidth: true
                            model: _countryModel
                            textRole: "country_code"
                            valueRole: "country_code"
                           currentIndex: currentProduct ? indexOfValue(currentProduct.countryCode) : indexOfValue("KE")
                        }

                        Label { text: "Product Type:"; font.bold: true }
                        ComboBox {
                            id: typeCombo
                            Layout.fillWidth: true
                            model: _productTypeModel
                            textRole: "type_code_name"
                            valueRole: "id"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.productTypeId, "type_code_name") : 0
                        }
                        Label { text: "Product Category:"; font.bold: true }
                        ComboBox {
                            id: catCombo
                            Layout.fillWidth: true
                            Layout.preferredHeight: 45

                            // Connect to the model provided by your root or controller
                            model: _productCategoryModel

                            // What the user sees
                            textRole: "product_category_name"

                            // What is actually saved to the DB
                            valueRole: "id"

                            // Ensure it selects the current category when editing
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.productCategoryId, "id") : -1

                            displayText: currentIndex === -1 ? "Select Category..." : currentText
                        }



                        Label { text: "Tax Type:"; font.bold: true }
                        ComboBox {
                            id: taxCombo
                            Layout.fillWidth: true
                            model: _taxClassificationModel
                            textRole: "tax_type_code"
                            valueRole: "tax_type_code"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.taxClassificationCode, "tax_type_code") : 0
                        }
                        // 2. Display the Tax Rate
                        Label { text: "Tax Rate:"; font.bold: true }
                        Text {
                            id: textRate
                            // Use a function or a direct binding to the currentValue of the combo
                            property real rate: {
                                if (taxCombo.currentValue !== undefined) {
                                    return _taxClassificationModel.getValueById(taxCombo.currentValue, "tax_percentage_rate")
                                }
                                return 0.0
                            }

                            text: rate + "%"
                            font.bold: true
                            color: "#2E7D32"
                        }


                        Button {
                            text: "Save Product Information"
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            Layout.topMargin: 10
                            highlighted: true
                            onClicked: {
                                // IMPORTANT: The keys here MUST match the controller's data.value("key")
                                let payload = {
                                    "localId": -1,
                                    "id": currentProduct ? currentProduct.id : "",
                                    "internalProductName": nameIn.text,
                                    "productCategoryId": catCombo.currentValue,
                                    "kraItemCode": kraIn.text,
                                    // "inventoryProductId": invCombo.currentValue,
                                    "productTypeId": typeCombo.currentValue,
                                    "currencyCode": currencyCombo.currentText,     // MUST NOT BE EMPTY
                                    "countryCode": countryCombo.currentText, // MUST NOT BE EMPTY
                                    "defaultSellingPrice": parseFloat(priceIn.text) || 0.0,
                                    "taxClassificationCode": taxCombo.currentText,
                                    "taxRate": textRate.rate
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
                    visible: currentProduct !== null

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
                                    Layout.preferredHeight: 45
                                    model: menuSetupRoot.inventoryModel
                                    textRole: "name"
                                    valueRole: "id"
                                    currentIndex: currentProduct ? indexOfValue(currentProduct.inventoryId) : -1
                                    displayText: currentIndex === -1 ? "Select Inventory Item..." : currentText
                                    onActivated: (index) => {
                                                     let selectedId = valueAt(index)
                                                     console.log("Linking Product to Inventory ID:", selectedId)

                                                     // If your model tracks the active ID, update it here
                                                     menuSetupRoot.inventoryModel.inventoryId = selectedId
                                                 }
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
                                    model: _quantityUnitModel
                                    textRole: "quantity_unit_code_name"
                                    valueRole: "quantity_unit_code_name"
                                    currentIndex: currentProduct ? findIndexByValue(model, currentProduct.unitId, "quantity_unit_code_name") : 0
                                }
                            }

                            Button {
                                text: "Add"
                                highlighted: true
                                Layout.alignment: Qt.AlignBottom
                                enabled: ingSelector.currentIndex >= 0 && ingQty.text.length > 0
                                onClicked: {
                                    // console.log("ing", )
                                    controller.saveIngredient({
                                                                  "productId": currentProduct.id,
                                                                  "ingredientProductId": ingSelector.currentValue,
                                                                  "quantity": parseFloat(ingQty.text) || 1,
                                                                  "unitId": ingUnitCombo.currentValue
                                                              })
                                    ingQty.text = "";
                                    ingSelector.currentIndex = 0;
                                    ingUnitCombo.currentIndex = 0;
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
                                        onClicked: controller.removeIngredient(model.id)
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
