import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"

    // SalesViewController {
    //     id: _salesModel
    //     onOrderChanged: {
    //             console.log("UI DEBUG: Total Amount updated to:", totalAmount)
    //         }

    //         onItemCountChanged: {
    //             console.log("UI DEBUG: Item count is now:", itemCount)
    //         }
    // }
    property SalesViewController _salesModel

    ProductViewController { id: _productModel}


    PaymentController {
        id: payCtrl
        onPaymentFinished: (success, ref) => {
                               if (success) {
                                   paymentLoader.item.close()
                                   _salesModel.clearOrder()
                                   paymentLoader.sourceComponent = null
                               }
                           }
    }

    Loader {
        id: paymentLoader
        anchors.fill: parent
        active: false
        sourceComponent: Component {
            PaymentDialog {
                // _salesModel: root._salesModel
                paymentCtrl: payCtrl
                onClosed: paymentLoader.active = false
            }
        }
    }

    Loader {
        id: modifierLoader
        active: false
        source: "sales/ModifierDialog.qml"
        anchors.fill: parent

        // This is the bridge variable
        property var currentModifierData: []
        property var currentItemContext: null

        onLoaded: {
            // 1. Assign the data strictly from our bridge variable
            item.availableModifiers = currentModifierData;

            // 2. Add a small delay or ensure the item is ready before opening
            item.open();

            // 3. Connect signals
            item.modifiersSelected.connect((selectedMods) => {
                                               _salesModel.addWithModifiers(currentItemContext, selectedMods);
                                               active = false;
                                           });

            item.rejected.connect(() => active = false);
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // --- Left Side: Item Grid ---
        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 400
            color: window.theme.background

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Label {
                    text: "Menu Items"
                    font.pixelSize: 28
                    font.bold: true
                    color: window.theme.textMain
                }

                // --- Category Bar ---
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "transparent"

                    ListView {
                        id: categoryBar
                        anchors.fill: parent
                        orientation: ListView.Horizontal
                        spacing: 12
                        clip: true
                        model: _productCategoryModel

                        delegate: Button {
                            id: catButton
                            text: model.product_category_name

                            contentItem: Text {
                                text: catButton.text
                                font.bold: true
                                color: _productModel.currentCategory === text ? window.theme.textMain : window.theme.textSecondary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 38
                                color: _productModel.currentCategory === catButton.text ? window.theme.accent : window.theme.surface
                                radius: 20
                                border.color: _productModel.currentCategory === catButton.text ? window.theme.accent : window.theme.border
                            }

                            onClicked: {
                                _productModel.currentCategory = model.product_category_name
                                categoryBar.positionViewAtIndex(index, ListView.Center);
                            }
                        }
                    }
                }

                GridView {
                    id: grid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: _salesModel.productModel
                    cellWidth: 230
                    cellHeight: 230

                    delegate: Card {
                        itemName: model.internalProductName
                        price: model.priceFormatted
                        width: grid.cellWidth - 30
                        height: grid.cellHeight - 30
                        onClicked: {
                            let data = model;
                            if (data.availableModifiers && data.availableModifiers.length > 0) {
                                // SET DATA FIRST
                                modifierLoader.currentModifierData = data.availableModifiers;
                                modifierLoader.currentItemContext = data;
                                // THEN ACTIVATE
                                modifierLoader.active = true;
                            } else {
                                _salesModel.addItem(model.id);
                            }
                        }

                    }
                }
            }
        }

        // --- Right Side: Persistent Order Panel ---
        Rectangle {
            id: orderSidePanel
            SplitView.preferredWidth: 350
            SplitView.minimumWidth: 300
            color: window.theme.sidePanelBg
            border.color: window.theme.border

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10
                // Dim the content slightly when busy
                opacity: (_salesModel && _salesModel.isBusy) ? 0.5 : 1.0
                enabled: !_salesModel || !_salesModel.isBusy

                Text {
                    text: "Current Order"
                    font.pixelSize: 22
                    font.bold: true
                    color: window.theme.textMain
                }

                ListView {
                    id: orderList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: _salesModel.orderModel
                    clip: true
                    spacing: 8

                    delegate: ItemDelegate {
                        id: orderItemRoot
                        width: orderList.width
                        padding: 10
                        Component.onCompleted: {
                            console.log("DEBUG: Row Index:", index)
                            console.log("DEBUG: Name Role Value:", model.name)
                            console.log("DEBUG: Unit Price Value:", model.unitPrice)
                        }
                        contentItem: RowLayout {
                            spacing: 12

                            Button {
                                text: "×"
                                font.pixelSize: 18
                                palette.buttonText: window.theme.danger
                                flat: true
                                Layout.preferredWidth: 30
                                onClicked: _salesModel.removeItem(index)
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: model.name
                                    font.bold: true
                                    color: window.theme.textMain
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                ColumnLayout {
                                    id: modifierContainer
                                    Layout.fillWidth: true
                                    spacing: 1
                                    property var parsedModifiers: model.modifiers ? JSON.parse(model.modifiers) : []
                                    visible: parsedModifiers.length > 0

                                    Repeater {
                                        model: modifierContainer.visible ? orderItemRoot.itemData.modifiers : []
                                        delegate: Text {
                                            // Shows: "+ Add-on Name (1.00 Ksh.)"
                                            text: "+ " + modelData.name + " (" + (modelData.price_cents / 100).toFixed(2) + " Ksh.)"
                                            font.pixelSize: 11
                                            color: window.theme.success  // Green color for add-ons
                                            font.italic: true
                                            Layout.leftMargin: 10
                                        }
                                    }
                                }
                                Text {
                                    text: model.totalPrice + " Ksh."
                                    color: window.theme.textSecondary
                                    font.pixelSize: 12
                                }
                            }

                            RowLayout {
                                spacing: 5
                                Button {
                                    text: "-"
                                    Layout.preferredWidth: 30
                                    onClicked: {
                                        if (model.quantity > 1) {
                                            _salesModel.updateQuantity(index, model.quantity - 1)
                                        } else {
                                            _salesModel.removeItem(index)
                                        }
                                    }
                                }
                                Text {
                                    text: model.quantity
                                    color: window.theme.textMain
                                    font.bold: true
                                    Layout.preferredWidth: 20
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Button {
                                    text: "+"
                                    Layout.preferredWidth: 30
                                    onClicked: _salesModel.updateQuantity(index, model.quantity + 1)
                                }
                            }
                        }

                        background: Rectangle {
                            color: hovered ? window.theme.surfaceHighlight : window.theme.surface
                            radius: 8
                            border.color: window.theme.border
                        }
                    }
                }

                // --- Totals Summary ---
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8


                    // Read values from the controller for cleaner calculations
                    readonly property double currentTotal: _salesModel.totalAmount
                    // Net = Total / 1.16 (assuming 16% VAT is inclusive)
                    readonly property double netAmount: currentTotal / 1.16
                    readonly property double taxValue: currentTotal - netAmount

                    Rectangle { Layout.fillWidth: true; height: 1; color: window.theme.border; opacity: 0.5 }

                    RowLayout {
                        Text { text: "Sub-total"; color: window.theme.textSecondary; font.pixelSize: 13 }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: _salesModel.itemCount > 0 ? _salesModel.totalAmount + " Ksh." : "--.--"
                            color: window.theme.textSecondary;
                            font.pixelSize: 13
                        }
                    }

                    RowLayout {
                        Text { text: "Tax (16%)"; color: window.theme.textSecondary; font.pixelSize: 13 }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: _salesModel.itemCount > 0 ? _salesModel.totalTaxFormatted + " Ksh." : "--.--"
                            color: window.theme.textSecondary;
                            font.pixelSize: 13
                        }
                    }

                    RowLayout {
                        Layout.topMargin: 5
                        Text { text: "Total Amount"; color: window.theme.textMain; font.bold: true; font.pixelSize: 18 }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: _salesModel.itemCount > 0 ? _salesModel.totalAmount + " Ksh." : "--.--"
                            color: window.theme.success;
                            font.bold: true;
                            font.pixelSize: 20
                        }
                    }
                }
                // --- Action Buttons ---
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Layout.topMargin: 5

                    // PLACE ORDER BUTTON
                    Button {
                        id: orderButton
                        text: "PLACE ORDER"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50

                        // Disable if busy, if model is null, or if cart is empty
                        enabled: !!_salesModel && !_salesModel.isBusy && _salesModel.itemCount > 0

                        contentItem: Text {
                            text: orderButton.text
                            color: "white"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: orderButton.enabled ? window.theme.success : "#34495e"
                            radius: 6
                            // Subtle visual feedback for press
                            opacity: orderButton.pressed ? 0.8 : 1.0
                        }

                        onClicked: {
                            if (_salesModel.makeOrder()) {
                                paymentLoader.active = true
                                paymentLoader.item.open()
                            }
                        }
                    }

                    // CANCEL ORDER BUTTON
                    Button {
                        id: clearButton
                        text: "CANCEL ORDER"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        enabled: !!_salesModel && !_salesModel.isBusy && _salesModel.itemCount > 0

                        contentItem: Text {
                            text: clearButton.text
                            color: window.theme.danger
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: "transparent"
                            border.color: window.theme.danger
                            border.width: 1
                            radius: 6
                            // Fade out the border when disabled
                            opacity: clearButton.enabled ? 1.0 : 0.3
                        }

                        onClicked: _salesModel.clearOrder()
                    }
                }
            }
            // Overlay Layer (BusyIndicator)
                BusyIndicator {
                    id: loadingIcon
                    anchors.centerIn: parent
                    z: 10
                    // Use optional chaining or a simple check to prevent errors if _salesModel is null
                    running: !!_salesModel && _salesModel.isBusy
                    visible: running

                    // Optional: Add a smooth fade-in animation for the indicator
                    OpacityAnimator on opacity {
                        from: 0; to: 1; duration: 200
                        running: loadingIcon.visible
                    }
                }
        }
    }
}
