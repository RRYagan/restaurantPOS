import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: root
    color: "#f8f9fa"
    property SalesModel salesModel: null

    MenuModel { id: menuModel }

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
                        text: (salesModel ? salesModel.totalFormatted : "0.00") + " €"
                        font.bold: true; font.pixelSize: 18; color: "#27ae60"
                    }
                }

                Button {
                    text: "Make Order"
                    highlighted: true
                    Layout.fillWidth: true
                    // enabled: salesModel && salesModel.rowCount() > 0
                    onClicked: {
                        if (salesModel.makeOrder()) {
                            console.log("Order saved to database successfully.");
                        }
                    }
                }

                Button {
                    text: "Clear Cart"
                    // flat: true
                    Layout.fillWidth: true
                    onClicked: salesModel.clearOrder()
                }
                // Button {
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
