import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
// import POS.Menu 1.0
import POS.Sales 1.0

Rectangle {
    id: root
    color: "#f8f9fa"



    property SalesModel salesModel: null

    CategoryModel { id: catModel }
    MenuModel {
        id: menuModel
        currentCategory: "All"
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
                        clip: true // Prevents categories from drawing outside the bounds while sliding

                        // Hide scrollbar for a cleaner mobile/modern look
                        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOff }

                        model: catModel

                        delegate: Button {
                            id: catButton
                            text: modelData
                            padding: 15

                            // Highlight logic using the property added to MenuModel
                            contentItem: Text {
                                text: catButton.text
                                font.bold: true
                                color: menuModel.currentCategory === modelData ? "white" : "#2c3e50"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 38
                                color: menuModel.currentCategory === modelData ? "#3498db" : "#ecf0f1"
                                radius: 20

                                // Subtle border for unselected items
                                border.color: menuModel.currentCategory === modelData ? "#2980b9" : "#dcdde1"
                                border.width: 1
                            }

                            onClicked: {
                                menuModel.currentCategory = modelData
                                // Optional: Smoothly scroll the clicked category into view
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
                    model: menuModel
                    cellWidth: 180
                    cellHeight: 200

                    delegate: Card {
                        itemName: model.name
                        priceCents: model.base_price_cents

                        onClicked: {
                            if (salesModel !== null) {
                                // Logic: This only updates the memory list, not the DB
                                salesModel.addItemToOrder(model.id);
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
            color: "white"
            border.color: "#e0e0e0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: "Current Order"
                    font.pixelSize: 22
                    font.bold: true
                }
                Label {
                    // Dynamically change title based on whether we are viewing an old order or a new one
                    text:  "Viewing Order"

                    font.pixelSize: 28
                    font.bold: true
                }

                // Inside MenuScreen.qml - Right Side SplitView Panel
                ListView {
                    id: orderList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: salesModel
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
                                palette.buttonText: "#e74c3c" // Red color
                                flat: true
                                Layout.preferredWidth: 30
                                onClicked: salesModel.removeItem(index)
                            }

                            // 2. Item Name and Price
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: model.name
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: (model.price ? model.price.formatted : "0.00") + " Ksh."
                                    font.pixelSize: 12
                                    color: "#7f8c8d"
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
                                        if (model.quantity > 1) {
                                            salesModel.updateQuantity(index, model.quantity - 1)
                                        } else {
                                            salesModel.removeItem(index)
                                        }
                                    }
                                }

                                Text {
                                    text: model.quantity
                                    font.bold: true
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.preferredWidth: 20
                                }

                                Button {
                                    text: "+"
                                    // flat: true
                                    Layout.preferredWidth: 30
                                    onClicked: salesModel.updateQuantity(index, model.quantity + 1)
                                }
                            }
                        }

                        // Add a subtle background highlight
                        background: Rectangle {
                            color: hovered ? "#f1f2f6" : "transparent"
                            radius: 4
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

                // Inside MenuScreen.qml - Right Side Panel objectName: "makeOrderButton"=
                // Inside MenuScreen.qml
                // This layout contains both buttons, ensuring they share the width properly
                // Inside MenuScreen.qml - Right Panel ColumnLayout
                // Inside the right-side panel ColumnLayout
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

                    // --- Clear Cart Button ---
                    Button {
                        id: clearButton
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40

                        contentItem: Text {
                            text: "CLEAR CART"
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
