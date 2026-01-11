import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0
import "sales"
Rectangle {
    id: root
    color: "transparent"

    // property MenuViewController salesModel: null
    property string currentOrderId: ""
    property string currentCategory: "All"

    // Tax Logic (Matching PaymentDialog 16% VAT)
    readonly property double totalVal: {
        if (!salesModel || !salesModel.totalFormatted) return 0.0;
        let clean = salesModel.totalFormatted.replace(/[^0-9.]/g, '');
        return parseFloat(clean) || 0.0;
    }
    readonly property double subTotal: totalVal / 1.16
    readonly property double taxAmount: totalVal - subTotal

    // MenuViewController { id: catModel }
    MenuViewController { id: menuModel }
    OrderViewController { id: salesModel }

    onCurrentCategoryChanged: {
        if (menuModel) {
            menuModel.currentCategory = currentCategory;
        }
    }

    PaymentController {
        id: payCtrl
        onPaymentFinished: (success, ref) => {
                               if (success) {
                                   paymentLoader.item.close()
                                   salesModel.clearOrder()
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
                salesModel: root.salesModel
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
                salesModel.addWithModifiers(currentItemContext, selectedMods);
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
                        model: menuModel.categoryModel

                        delegate: Button {
                            id: catButton
                            text: model.name

                            contentItem: Text {
                                text: catButton.text
                                font.bold: true
                                color: menuModel.currentCategory === text ? window.theme.textMain : window.theme.textSecondary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 38
                                color: menuModel.currentCategory === catButton.text ? window.theme.accent : window.theme.surface
                                radius: 20
                                border.color: menuModel.currentCategory === catButton.text ? window.theme.accent : window.theme.border
                            }

                            onClicked: {
                                menuModel.currentCategory = model.name
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
                    model: menuModel.model
                    cellWidth: 230
                    cellHeight: 230

                    delegate: Card {
                        itemName: model.name
                        priceCents: model.basePriceCents
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
                                salesModel.addItem({
                                        "id": model.id,
                                        "name": model.name,
                                        "price": model.basePriceCents,
                                        "taxType": model.taxType
                                    })
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
            // Dim the content slightly when busy
            opacity: (salesModel && salesModel.isBusy) ? 0.5 : 1.0
            enabled: !salesModel || !salesModel.isBusy

            BusyIndicator {
                id: loadingIcon
                anchors.centerIn: parent
                z: 10 // Ensure it sits above the list
                running: salesModel ? salesModel.isBusy : false
                visible: running
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10



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
                    model: salesModel
                    clip: true
                    spacing: 8

                    delegate: ItemDelegate {
                        id: orderItemRoot
                        width: orderList.width
                        padding: 10

                        property var itemData: model

                        contentItem: RowLayout {
                            spacing: 12

                            Button {
                                text: "×"
                                font.pixelSize: 18
                                palette.buttonText: window.theme.danger
                                flat: true
                                Layout.preferredWidth: 30
                                onClicked: salesModel.removeItem(index)
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
                                    visible: !!orderItemRoot.itemData &&
                                             !!orderItemRoot.itemData.modifiers &&
                                             orderItemRoot.itemData.modifiers.length > 0
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
                                    text: (model.price ? model.price.formatted : "-.--") + " Ksh."
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
                                            salesModel.updateQuantity(index, model.quantity - 1)
                                        } else {
                                            salesModel.removeItem(index)
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
                                    onClicked: salesModel.updateQuantity(index, model.quantity + 1)
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

                    Rectangle { Layout.fillWidth: true; height: 1; color: window.theme.border; opacity: 0.5 }

                    RowLayout {
                        Text { text: "Sub-total"; color: window.theme.textSecondary; font.pixelSize: 13 }
                        Item { Layout.fillWidth: true }
                        Text { text: root.subTotal ? (root.subTotal.toFixed(2) + " Ksh.") : "--.-- "; color: window.theme.textSecondary; font.pixelSize: 13 }
                    }

                    RowLayout {
                        Text { text: "Tax (16%)"; color: window.theme.textSecondary; font.pixelSize: 13 }
                        Item { Layout.fillWidth: true }
                        Text { text: root.taxAmount ? root.taxAmount.toFixed(2) + " Ksh." : "--.-- "; color: window.theme.textSecondary; font.pixelSize: 13 }
                    }

                    RowLayout {
                        Layout.topMargin: 5
                        Text { text: "Total Amount"; color: window.theme.textMain; font.bold: true; font.pixelSize: 18 }
                        Item { Layout.fillWidth: true }
                        Text { text: (salesModel && salesModel.itemCount > 0 ? salesModel.totalFormatted + " Ksh." : "--.--") ;
                        color: window.theme.success; font.bold: true; font.pixelSize: 20 }
                    }
                }

                // --- Action Buttons ---
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Layout.topMargin: 5

                    Button {
                        id: orderButton
                        text: "PLACE ORDER"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        enabled: salesModel !== null && salesModel.itemCount > 0

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
                        }
                        onClicked: {
                            if (salesModel.makeOrder()) {
                                paymentLoader.active = true
                                paymentLoader.item.open()
                            }
                        }
                    }

                    Button {
                        id: clearButton
                        text: "CANCEL ORDER"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        enabled: salesModel !== null && salesModel.itemCount > 0

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
                            radius: 6
                            opacity: clearButton.enabled ? 1.0 : 0.3
                        }
                        onClicked: salesModel.clearOrder()
                    }
                }
            }
        }
    }
}
