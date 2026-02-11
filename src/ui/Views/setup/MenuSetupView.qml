import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Item {
    id: menuSetupRoot
    property bool isEditing: false
    property var currentProduct: null

    ProductViewController {
        id : _productModel
    }
    InventoryViewController {
        id: _inventoryModel
    }

    // Background for the entire page
    Rectangle {
        anchors.fill: parent
        color: "#F8F9FA"
    }

    // Helper to find index for ComboBoxes (Logic Maintained)
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

            // Modern Header Area
            Rectangle {
                Layout.fillWidth: true
                height: 80
                color: "white"

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#EEEEEE"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    // Enhanced Search Bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 45
                        color: "#F1F3F4"
                        radius: 8
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15
                            Text { text: "🔍"; font.pixelSize: 14; opacity: 0.5 }
                            TextField {
                                id: searchBar
                                placeholderText: "Search products..."
                                Layout.fillWidth: true
                                background: null
                                font.pixelSize: 14
                                verticalAlignment: Text.AlignVCenter
                                // Note: Keeping original search behavior (none defined in original text, purely visual update)
                            }
                        }
                    }

                    Button {
                        text: "Add Product"
                        contentItem: Text {
                            text: parent.text
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitWidth: 120
                            implicitHeight: 45
                            color: parent.pressed ? "#1565C0" : "#1976D2"
                            radius: 8
                        }
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
                model: _productModel.productModel
                clip: true
                spacing: 10
                topMargin: 15

                delegate: ItemDelegate {
                    width: productList.width - 40
                    x: 20
                    height: 80

                    background: Rectangle {
                        color: parent.pressed ? "#F5F5F5" : "white"
                        radius: 10
                        border.color: parent.hovered ? "#BDBDBD" : "#EEEEEE"
                        layer.enabled: true
                        // Basic shadow effect
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 15

                        Rectangle {
                            width: 45; height: 45; radius: 8; color: "#E3F2FD"
                            Text { anchors.centerIn: parent; text: "📦"; font.pixelSize: 22 }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: model.internalProductName
                                font.bold: true; font.pixelSize: 15; color: "#212121"
                            }
                            RowLayout {
                                spacing: 10
                                Text { text: "Cat: " + model.productCategoryId; color: "#757575"; font.pixelSize: 12 }
                                Text { text: "| Origin: " + model.countryCode; color: "#9E9E9E"; font.pixelSize: 12 }
                            }
                        }

                        ColumnLayout {
                            Layout.alignment: Qt.AlignRight
                            spacing: 0
                            Text {
                                text: model.currencyCode+ " " + parseFloat(model.defaultSellingPrice).toFixed(2)
                                color: "#2E7D32"; font.bold: true; font.pixelSize: 16
                            }
                            Text {
                                text: "+" + parseFloat(model.taxAmount).toFixed(2) + " Tax"
                                color: "#757575"; font.pixelSize: 11
                                visible: model.taxAmount > 0
                                Layout.alignment: Qt.AlignRight
                            }
                        }
                    }

                    onClicked: {
                        _productModel.productId = model.id
                        menuSetupRoot.currentProduct = {
                            "id": model.id,
                            "internalProductName": model.internalProductName,
                            "productCategoryId": model.productCategoryId,
                            "kraItemCode": model.kraItemCode,
                            "productTypeId": model.productTypeId,
                            "currencyCode": model.currencyCode,
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
                width: parent.width - 60
                x: 30
                spacing: 25

                // Editor Toolbar
                RowLayout {
                    Layout.topMargin: 20
                    Button {
                        text: "← Back to List"
                        flat: true
                        onClicked: menuSetupRoot.isEditing = false
                    }
                    Text {
                        text: currentProduct ? "Edit Product: " + currentProduct.internalProductName : "New Product"
                        font.pixelSize: 24; font.bold: true; color: "#212121"
                    }
                    Item { Layout.fillWidth: true }
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
                        rowSpacing: 15
                        columnSpacing: 30

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

                        Label { text: "Price:"; font.bold: true }
                        TextField {
                            id: priceIn
                            text: currentProduct ? currentProduct.defaultSellingPrice : ""
                            Layout.fillWidth: true
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                        }

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
                            valueRole: "type_code"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.productTypeId, "type_code_name") : 0
                        }

                        Label { text: "Product Category:"; font.bold: true }
                        ComboBox {
                            id: catCombo
                            Layout.fillWidth: true
                            model: _productCategoryModel
                            textRole: "product_category_name"
                            valueRole: "id"
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

                        Label { text: "Tax Rate:"; font.bold: true }
                        Text {
                            id: textRate
                            property real rate: {
                                if (taxCombo.currentValue !== undefined) {
                                    return _taxClassificationModel.getValueById(taxCombo.currentValue, "tax_percentage_rate")
                                }
                                return 0.0
                            }
                            text: rate + "%"
                            font.bold: true; color: "#2E7D32"
                        }

                        Button {
                            text: "Save Product Information"
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            Layout.topMargin: 15
                            highlighted: true
                            onClicked: {
                                let payload = {
                                    "localId": -1,
                                    "id": currentProduct ? currentProduct.id : "",
                                    "internalProductName": nameIn.text,
                                    "productCategoryId": catCombo.currentValue,
                                    "kraItemCode": kraIn.text,
                                    "productTypeId": typeCombo.currentValue,
                                    "currencyCode": currencyCombo.currentText,
                                    "countryCode": countryCombo.currentText,
                                    "defaultSellingPrice": parseFloat(priceIn.text) || 0.0,
                                    "taxClassificationCode": taxCombo.currentText,
                                    "taxRate": textRate.rate
                                }

                                if (_productModel.saveProduct(payload)) {
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
                        font.bold: true; font.pixelSize: 20; color: "#212121"
                    }

                    // A. ADD INGREDIENT FORM
                    Rectangle {
                        Layout.fillWidth: true
                        height: 100
                        color: "white"
                        border.color: "#E0E0E0"
                        radius: 12

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 15

                            ColumnLayout {
                                Layout.fillWidth: true
                                Label { text: "Select Item"; font.pixelSize: 11; color: "#666" }
                                ComboBox {
                                    id: ingSelector
                                    Layout.fillWidth: true
                                    model: _inventoryModel.inventoryModel
                                    textRole: "name"
                                    valueRole: "id"
                                    onActivated: (index) => {
                                        _inventoryModel.inventoryId = valueAt(index)
                                    }
                                }
                            }

                            ColumnLayout {
                                width: 80
                                Label { text: "Qty"; font.pixelSize: 11; color: "#666" }
                                TextField { id: ingQty; placeholderText: "0.0"; Layout.fillWidth: true }
                            }

                            ColumnLayout {
                                width: 100
                                Label { text: "Unit"; font.pixelSize: 11; color: "#666" }
                                Text {
                                    text: _inventoryModel.selectedUnitName || "---"
                                    font.bold: true; color: "#1976D2"
                                    visible: ingSelector.currentIndex !== -1
                                }
                            }

                            Button {
                                text: "Add"
                                highlighted: true
                                Layout.alignment: Qt.AlignBottom
                                enabled: ingSelector.currentIndex >= 0 && ingQty.text.length > 0
                                onClicked: {
                                    let details = _inventoryModel.getInventoryDetails(ingSelector.currentValue);
                                    let selectedUnitId = details.quantityUnitId;

                                    _productModel.saveIngredient({
                                        "productId": currentProduct.id,
                                        "ingredientProductId": ingSelector.currentValue,
                                        "quantity": parseFloat(ingQty.text) || 1,
                                        "unitId": selectedUnitId
                                    });

                                    ingQty.text = "";
                                    ingSelector.currentIndex = -1;
                                }
                            }
                        }
                    }

                    // B. INGREDIENTS LIST
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Repeater {
                            model: _productModel.compositionModel
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 60
                                color: "white"
                                radius: 10
                                border.color: "#EEEEEE"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 15
                                    spacing: 15

                                    Rectangle {
                                        width: 50; height: 30; color: "#E8F5E9"; radius: 6
                                        Text {
                                            anchors.centerIn: parent
                                            text: model.quantity
                                            color: "#2E7D32"; font.bold: true
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
                                            text: "Item ID: " + model.ingredientProductId
                                            font.pixelSize: 11; color: "#9E9E9E"
                                        }
                                    }

                                    Button {
                                        text: "Remove"
                                        flat: true
                                        contentItem: Text {
                                            text: "Remove"
                                            color: "#D32F2F"
                                            font.bold: true
                                        }
                                        onClicked: _productModel.removeIngredient(model.id)
                                    }
                                }
                            }
                        }

                        Text {
                            text: "No ingredients added to this recipe yet."
                            visible: _productModel.compositionModel.count === 0
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 20
                            color: "#999"
                            font.italic: true
                        }
                    }
                }

                Item { height: 40; Layout.fillWidth: true }
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
