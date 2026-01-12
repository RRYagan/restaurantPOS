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
            Rectangle {
                Layout.fillWidth: true; height: 70; color: whiteTheme.surface
                RowLayout {
                    anchors.fill: parent; anchors.margins: 15
                    TextField {
                        id: searchBar; placeholderText: "Filter menu items..."; Layout.fillWidth: true
                        background: Rectangle { radius: 10; color: whiteTheme.background; border.color: whiteTheme.border }
                    }
                    Button {
                        text: "Add Product"
                        highlighted: true
                        onClicked: { menuSetupRoot.currentProduct = null; menuSetupRoot.isEditing = true }
                    }
                }
            }

            ListView {
                id: productList
                Layout.fillWidth: true; Layout.fillHeight: true
                model: controller.productModel
                clip: true; spacing: 5
                delegate: ItemDelegate {
                    width: productList.width - 20; x: 10; height: 65
                    background: Rectangle { color: hovered ? whiteTheme.surfaceHighlight : whiteTheme.surface; radius: 8; border.color: whiteTheme.border }
                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true
                            Text { text: model.name; font.bold: true; color: whiteTheme.textMain }
                            Text { text: "Cat: " + model.category; color: whiteTheme.textSecondary; font.pixelSize: 11 }
                        }
                        Text { text: "$" + parseFloat(model.price).toFixed(2); color: whiteTheme.accent; font.bold: true }
                    }
                    onClicked: {
                        controller.productId = model.id
                        menuSetupRoot.currentProduct = {
                            "id": model.id,
                            "localId": model.localId,
                            "name": model.name,
                            "price": model.price,
                            "kraCode": model.kraCode,
                            "categoryCode": model.category, // Map from model roles
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
            ColumnLayout {
                width: parent.width - 40; x: 20; spacing: 20

                RowLayout {
                    Layout.topMargin: 20
                    Button { text: "← Cancel"; flat: true; onClicked: menuSetupRoot.isEditing = false }
                    Text { text: "Product Details"; font.pixelSize: 20; font.bold: true; color: whiteTheme.textMain }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: editGrid.implicitHeight + 40
                    color: whiteTheme.surface; radius: 12; border.color: whiteTheme.border


                    GridLayout {
                        id: editGrid
                        anchors.fill: parent; anchors.margins: 20
                        columns: 2; rowSpacing: 15; columnSpacing: 20

                        Label { text: "Name:" }
                        TextField { id: nameIn; text: currentProduct ? currentProduct.name : "test"; Layout.fillWidth: true }

                        Label { text: "Price ($):" }
                        TextField { id: priceIn; text: currentProduct ? currentProduct.price : "10"; Layout.fillWidth: true }

                        Label { text: "KRA Code:" }
                        TextField { id: kraIn; text: currentProduct ? currentProduct.kraCode : "KRA3445"; Layout.fillWidth: true }

                        Label { text: "Category:" }
                        ComboBox {
                            id: catCombo
                            Layout.fillWidth: true
                            model: menuSetupRoot.categoryModel
                            textRole: "text"
                            valueRole: "code"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.categoryCode, "code") : 0
                        }

                        Label { text: "Tax Classification:" }
                        ComboBox {
                            id: taxCombo
                            Layout.fillWidth: true
                            model: menuSetupRoot.taxModel
                            textRole: "text"
                            valueRole: "valueId"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.taxId, "valueId") : 0
                        }

                        Label { text: "Measurement Unit:" }
                        ComboBox {
                            id: unitCombo
                            Layout.fillWidth: true
                            model: menuSetupRoot.unitModel
                            textRole: "text"
                            valueRole: "valueId"
                            currentIndex: currentProduct ? findIndexByValue(model, currentProduct.unitId, "valueId") : 0
                        }



                        Button {
                            text: "Save Product"
                            Layout.columnSpan: 2; Layout.fillWidth: true; highlighted: true

                            enabled: nameIn.text.trim().length > 0

                            onClicked: {
                                console.log("Saving Product with Unit ID:", unitCombo.currentValue)
                                let payload = {
                                    "id": currentProduct ? currentProduct.id : "",
                                    "localId": currentProduct ? currentProduct.id : -1,
                                    "name": nameIn.text,
                                    "price": parseFloat(priceIn.text) || 0.0,
                                    "kraCode": kraIn.text,
                                    "categoryCode": catCombo.currentValue,
                                    "taxId": parseInt(taxCombo.currentValue),
                                    "unitId": parseInt(unitCombo.currentValue) // Ensure this is an Int
                                }

                                console.log("payload unitId",payload.unitId)
                                console.log("payload name",payload.name)
                                console.log("payload taxID",payload.taxId)
                                console.log("payload price",payload.price)
                                console.log("payload kraCode",payload.kraCode)
                                console.log("payload categoryCode",payload.categoryCode)



                                if (controller.saveProduct(payload)) {
                                    menuSetupRoot.isEditing = false
                                } else {
                                    // Show a visual hint that it failed
                                    saveErrorAnim.start()
                                }
                            }
                        }


                        RowLayout {
                            spacing: 10
                            Layout.fillWidth: true

                            ComboBox {
                                id: ingSelector
                                Layout.fillWidth: true
                                model: controller.productModel
                                textRole: "name"
                                valueRole: "id"
                                currentIndex: -1
                                displayText: currentIndex === -1 ? "Select Ingredient..." : currentText
                            }

                            TextField {
                                id: ingQty
                                placeholderText: "Qty"
                                width: 80
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }

                            Button {
                                text: "Add"
                                highlighted: true
                                enabled: ingSelector.currentIndex !== -1 && ingQty.text !== ""
                                onClicked: {
                                    controller.compositionModel.addIngredient({
                                        "productId": currentProduct.id,
                                        "ingredientProductId": ingSelector.currentValue,
                                        "quantity": parseFloat(ingQty.text)
                                    })
                                    ingQty.text = ""
                                    ingSelector.currentIndex = -1
                                }
                            }
                        }
                        // Optional: Shake animation on error
                        SequentialAnimation on x {
                            id: saveErrorAnim
                            running: false
                            NumberAnimation { to: 15; duration: 50 }
                            NumberAnimation { to: 25; duration: 50 }
                            NumberAnimation { to: 20; duration: 50 }
                        }
                    }
                }
            }
        }
    }
}
