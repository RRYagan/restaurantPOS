import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left side: Menu Items
        Rectangle {
            SplitView.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                // Category Filter Bar
                ListView {
                    Layout.fillWidth: true
                    height: 50
                    orientation: ListView.Horizontal
                    model: catModel
                    spacing: 10
                    delegate: Button {
                        text: model.name
                        highlighted: menuModel.currentCategory === model.name
                        onClicked: menuModel.currentCategory = model.name
                    }
                }

                GridView {
                    id: menuGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cellWidth: 180; cellHeight: 200
                    model: menuModel
                    clip: true
                    delegate: Card {
                        itemName: model.name
                        price: model.base_price_cents // Passing the Money gadget
                        onClicked: salesModel.addItemToOrder(model.id)
                    }
                }
            }
        }

        // Right side: Current Cart
        Rectangle {
            SplitView.preferredWidth: 350
            color: "white"
            border.color: "#e0e0e0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15

                Text { text: "Current Order"; font.pixelSize: 22; font.bold: true }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: salesModel
                    clip: true
                    delegate: RowLayout {
                        width: parent.width
                        Text { text: model.quantity + "x"; Layout.preferredWidth: 30 }
                        Text { text: model.name; Layout.fillWidth: true }
                        Text { text: model.price.formatted }
                    }
                }

                Separator { Layout.fillWidth: true }

                RowLayout {
                    Text { text: "Total:"; font.bold: true; font.pixelSize: 20 }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: salesModel.totalFormatted + " Ksh"
                        font.bold: true; font.pixelSize: 20; color: "#27ae60"
                    }
                }

                Button {
                    text: "CHECKOUT"
                    Layout.fillWidth: true
                    palette.button: "#2ecc71"
                    enabled: salesModel.rowCount() > 0
                    onClicked: {
                        if (salesModel.makeOrder()) {
                            console.log("Order Saved Successfully");
                        }
                    }
                }
            }
        }
    }
}
