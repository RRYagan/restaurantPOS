import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
// import POS.Menu 1.0
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"



    property SalesView salesModel: null

    property string currentCategory: "All"
    CategoryView { id: catModel }
    MenuView { id: menuModel }
    onCurrentCategoryChanged: {
            if (menuModel.proxy) {
                menuModel.proxy.categoryFilter = currentCategory;
            }
        }


    property string currentOrderId: ""


    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // --- Left Side: Item Grid ---
        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 400
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "Menu Items"
                        font.pixelSize: 28
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                }

                // Inside the Left Side ColumnLayout, before the GridView
                Rectangle {
                    Layout.fillWidth: true
                    height: 50 // Fixed height for the scrollable bar
                    color: "transparent"

                    ListView {
                        id: categoryBar
                        anchors.fill: parent
                        orientation: ListView.Horizontal
                        spacing: 12
                        clip: true
                        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOff }

                        model: catModel.proxy

                        delegate: Button {
                            id: catButton
                            // FIX 1: Change modelData to model.displayData.name
                            text: model.displayData.name
                            padding: 15

                            contentItem: Text {
                                // FIX 2: Use the property from the map
                                text: model.displayData.name
                                font.bold: true
                                // FIX 3: Compare against the actual categoryFilter string
                                color: menuModel.proxy.categoryFilter === model.displayData.name ? "white" : "#2c3e50"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 38
                                // FIX 4: Correct property name (categoryFilter, not categoryFilterChanged)
                                color: menuModel.proxy.categoryFilter === model.displayData.name ? "#3498db" : "#ecf0f1"
                                radius: 20
                                border.color: menuModel.proxy.categoryFilter === model.displayData.name ? "#2980b9" : "#dcdde1"
                                border.width: 1
                            }

                            onClicked: {
                                // FIX 5: Update the filter using the name string from the map
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
                        // anchors.horizontalCenter: parent ? grid.horizontalCenter : undefined

                        onClicked: {
                            if (salesModel !== null) {
                                // Logic: This only updates the memory list, not the DB
                                // console.log(model.displayData.price_cents)
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
            color: "transparent"
            border.color: "#1affffff"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: "Current Order"
                    font.pixelSize: 22
                    font.bold: true
                }

                // Inside MenuScreen.qml - Right Side SplitView Panel
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

                            // 1. Remove Button on the left
                            Button {
                                text: "×"
                                font.pixelSize: 20
                                font.bold: true
                                palette.buttonText: "#ff7675"
                                flat: true
                                Layout.preferredWidth: 30
                                onClicked: salesModel.removeItem(index)
                            }

                            // 2. Item Name and Price
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: model.displayData.name
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                    color: "white"
                                }
                                Text {
                                    text: (model.displayData.price ? model.displayData.price.formatted : "0.00") + " Ksh."
                                    font.pixelSize: 12
                                    color: "#bdc3c7"
                                }
                            }

                            // 3. Quantity Controls (Fit to the right)
                            RowLayout {
                                spacing: 5

                                Button {
                                    text: "-"
                                    // flat: true
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
                                    font.bold: true
                                    font.pixelSize: 14
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.preferredWidth: 20
                                }

                                Button {
                                    text: "+"
                                    // flat: true
                                    Layout.preferredWidth: 30
                                    onClicked: salesModel.updateQuantity(index, model.displayData.quantity + 1)
                                }
                            }
                        }

                        // Add a subtle background highlight
                        background: Rectangle {
                            color: hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.08)
                                    radius: 6
                                    border.color: Qt.rgba(1, 1, 1, 0.1)
                                    border.width: 1
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#e0e0e0"
                    Layout.topMargin: 5
                    Layout.bottomMargin: 5
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Total:"; font.bold: true; font.pixelSize: 18 }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: (salesModel ? salesModel.totalFormatted : "0.00") + " Ksh."
                        font.bold: true; font.pixelSize: 18; color: "#27ae60"
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Tax:"; font.bold: true; font.pixelSize: 18 }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: (salesModel ? salesModel.totalFormatted : "0.00") + " Ksh."
                        font.bold: true; font.pixelSize: 18; color: "#27ae60"
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Final Total:"; font.bold: true; font.pixelSize: 18 }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: (salesModel ? salesModel.totalFormatted : "0.00") + " Ksh."
                        font.bold: true; font.pixelSize: 18; color: "#27ae60"
                    }
                }


                ColumnLayout {
                    objectName: "makeOrderButton"
                    id: buttonLayout
                    Layout.fillWidth: true
                    spacing: 10
                    Layout.topMargin: 10

                    // --- Place/Update Order Button ---
                    Button {
                        id: orderButton
                        Layout.fillWidth: true
                        // Cap the height to prevent it from looking oversized
                        Layout.preferredHeight: 45
                        Layout.maximumHeight: 50

                        contentItem: Text {
                            // Text changes based on whether m_currentOrderId is set
                            text: "PLACE ORDER"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 14

                            // Forces the text to shrink rather than expanding the button
                            fontSizeMode: Text.Fit
                            minimumPixelSize: 9
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 12
                            rightPadding: 12
                        }

                        background: Rectangle {
                            // Blue for Update, Green for New Order
                            color: orderButton.enabled
                                ? "#2ecc71"
                                : "#bdc3c7"
                            radius: 4
                        }

                        enabled: salesModel !== null && salesModel.totalFormatted !== "0.00"
                        onClicked: salesModel.makeOrder()
                    }

                    // --- Cancel order ---
                    Button {
                        id: clearButton
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40

                        contentItem: Text {
                            text: "CANCEL ORDER"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: clearButton.enabled ? "#e74c3c" : "#bdc3c7"
                            radius: 4
                        }

                        enabled: salesModel !== null && salesModel.totalFormatted !== "0.00"
                        onClicked: salesModel.clearOrder()
                    }
                }// Button {
                //         text: ">"
                //         font.bold: true
                //         flat: true
                //         enabled: salesModel && salesModel.rowCount() > 0
                //         onClicked: contentStack.push("PaymentPage.qml", { "salesModel": salesModel })
                //     }
            }
        }
    }
}
