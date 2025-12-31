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

                ListView {
                    id: orderList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: salesModel
                    clip: true

                    delegate: ItemDelegate {
                        width: orderList.width
                        contentItem: RowLayout {
                            Text {
                                text: model.quantity + "x " + model.name
                                Layout.fillWidth: true
                            }
                            Text {
                                text: (model.price ? model.price.formatted : "0.00") + " Ksh."
                                font.bold: true
                            }
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
            }
        }
    }
}
