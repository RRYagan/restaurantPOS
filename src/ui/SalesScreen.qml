import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"

    property SalesView salesModel: null
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
    // --- THEME PROPERTIES ---
    // These unify the feel with Main.qml and PaymentDialog.qml
    QtObject {
        id: theme
        property color background: "transparent"
        property color surface: Qt.rgba(1, 1, 1, 0.08)       // Transparent white for "glass" effect
        property color surfaceHighlight: Qt.rgba(1, 1, 1, 0.18)
        property color accent: "#3498db"                     // Blue focus color from Card.qml
        property color success: "#2ecc71"                    // Green for prices and "Place Order"
        property color danger: "#e74c3c"                     // Red for "Cancel Order"
        property color textMain: "#ffffff"
        property color textSecondary: "#95a5a6"              // Muted gray from PaymentDialog
        property color border: Qt.rgba(1, 1, 1, 0.15)
        property color sidePanelBg: Qt.rgba(0, 0, 0, 0.25)   // Slightly darker area for order list
    }


    CategoryView { id: catModel }
    MenuView { id: menuModel }

    onCurrentCategoryChanged: {
        if (menuModel.proxy) {
            menuModel.proxy.categoryFilter = currentCategory;
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

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // --- Left Side: Item Grid ---
        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 400
            color: theme.background

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Label {
                    text: "Menu Items"
                    font.pixelSize: 28
                    font.bold: true
                    color: theme.textMain
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
                        model: catModel.proxy

                        delegate: Button {
                            id: catButton
                            text: model.displayData.name

                            contentItem: Text {
                                text: catButton.text
                                font.bold: true
                                color: menuModel.proxy.categoryFilter === text ? theme.textMain : theme.textSecondary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 38
                                color: menuModel.proxy.categoryFilter === catButton.text ? theme.accent : theme.surface
                                radius: 20
                                border.color: menuModel.proxy.categoryFilter === catButton.text ? theme.accent : theme.border
                            }

                            onClicked: {
                                menuModel.proxy.categoryFilter = model.displayData.name
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
                    model: menuModel.proxy
                    cellWidth: 230
                    cellHeight: 230

                    delegate: Card {
                        itemName: model.displayData.name
                        priceCents: model.displayData.price_cents
                        width: grid.cellWidth - 30
                        height: grid.cellHeight - 30
                        onClicked: {
                            if (salesModel !== null) {
                                salesModel.addItemToOrder(model.displayData.id);
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
            color: theme.sidePanelBg
            border.color: theme.border
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
                    color: theme.textMain
                }

                ListView {
                    id: orderList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: salesModel.proxy
                    clip: true
                    spacing: 8

                    delegate: ItemDelegate {
                        width: orderList.width
                        padding: 10

                        contentItem: RowLayout {
                            spacing: 12

                            Button {
                                text: "×"
                                font.pixelSize: 18
                                palette.buttonText: theme.danger
                                flat: true
                                Layout.preferredWidth: 30
                                onClicked: salesModel.removeItem(index)
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: model.displayData.name
                                    font.bold: true
                                    color: theme.textMain
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: (model.displayData.price ? model.displayData.price.formatted : "0.00") + " Ksh."
                                    color: theme.textSecondary
                                    font.pixelSize: 12
                                }
                            }

                            RowLayout {
                                spacing: 5
                                Button {
                                    text: "-"
                                    Layout.preferredWidth: 30
                                    onClicked: {
                                        if (model.displayData.quantity > 1) {
                                            salesModel.updateQuantity(index, model.displayData.quantity - 1)
                                        } else {
                                            salesModel.removeItem(index)
                                        }
                                    }
                                }
                                Text {
                                    text: model.displayData.quantity
                                    color: theme.textMain
                                    font.bold: true
                                    Layout.preferredWidth: 20
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Button {
                                    text: "+"
                                    Layout.preferredWidth: 30
                                    onClicked: salesModel.updateQuantity(index, model.displayData.quantity + 1)
                                }
                            }
                        }

                        background: Rectangle {
                            color: hovered ? theme.surfaceHighlight : theme.surface
                            radius: 8
                            border.color: theme.border
                        }
                    }
                }

                // --- Totals Summary ---
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle { Layout.fillWidth: true; height: 1; color: theme.border; opacity: 0.5 }

                    RowLayout {
                        Text { text: "Sub-total"; color: theme.textSecondary; font.pixelSize: 13 }
                         Item { Layout.fillWidth: true }
                        Text { text: root.subTotal.toFixed(2) + " Ksh."; color: theme.textSecondary; font.pixelSize: 13 }
                                        }

                        RowLayout {
                            Text { text: "Tax (16%)"; color: theme.textSecondary; font.pixelSize: 13 }
                            Item { Layout.fillWidth: true }
                            Text { text: root.taxAmount.toFixed(2) + " Ksh."; color: theme.textSecondary; font.pixelSize: 13 }
                        }

                        RowLayout {
                            Layout.topMargin: 5
                            Text { text: "Total Amount"; color: theme.textMain; font.bold: true; font.pixelSize: 18 }
                            Item { Layout.fillWidth: true }
                            Text { text: (salesModel ? salesModel.totalFormatted : "0.00") + " Ksh."; color: theme.success; font.bold: true; font.pixelSize: 20 }
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
                        enabled: salesModel !== null && salesModel.totalFormatted !== "0.00"

                        contentItem: Text {
                            text: orderButton.text
                            color: "white"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: orderButton.enabled ? theme.success : "#34495e"
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
                        enabled: salesModel !== null && salesModel.totalFormatted !== "0.00"

                        contentItem: Text {
                            text: clearButton.text
                            color: theme.danger
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: "transparent"
                            border.color: theme.danger
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
