import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects


Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#fffdf0"
    radius: 0
    clip: true
    // property var _salesModel

    layer.enabled: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: "#80000000"
        shadowBlur: 10
        shadowVerticalOffset: 2
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 5

        // --- HEADER ---
        Label {
            text: "STORE RECEIPT"
            color: "black"
            font { bold: true; pixelSize: 20; family: "Courier New" }
            Layout.alignment: Qt.AlignHCenter
        }

        // --- METADATA ---
        GridLayout {
            columns: 2
            Layout.fillWidth: true
            Label { text: "ORDER #:"; color: "black"; font.family: "Courier New" }
            Label { text: _salesModel.orderModel.currentOrderId; color: "black"; Layout.alignment: Qt.AlignRight; font.family: "Courier New" }
        }

        Column {
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            spacing: 2

            Rectangle { width: parent.width; height: 1; color: "black" }
            Rectangle { width: parent.width; height: 1; color: "black" }
        }

        // --- ITEM BREAKDOWN HEADER ---
        RowLayout {
            Layout.fillWidth: true
            Label { text: "QTY"; font.bold: true; color: "black"; Layout.preferredWidth: 40; font.family: "Courier New" }
            Label { text: "ITEM"; font.bold: true; color: "black"; Layout.fillWidth: true; font.family: "Courier New" }
            Label { text: "UNIT PRICE"; font.bold: true; color: "black"; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight; font.family: "Courier New" }
            Label { text: "PRICE"; font.bold: true; color: "black"; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight; font.family: "Courier New" }
            Label { text: "TAX"; font.bold: true; color: "black"; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight; font.family: "Courier New" }

        }

        // --- DYNAMIC ITEMS LIST ---
        ListView {
            id: itemsListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: _salesModel.orderModel // Uses the SalesModel/OrderItemModel
            interactive: false // Keep it static like a real receipt
            delegate: RowLayout {
                width: itemsListView.width
                Label {
                    text: model.quantity
                    color: "black"
                    Layout.preferredWidth: 40
                    font.family: "Courier New"
                }
                Label {
                    text: model.name
                    color: "black"
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.family: "Courier New"
                }
                Label {
                    text: model.unitPrice
                    color: "black"
                    Layout.preferredWidth: 80
                    horizontalAlignment: Text.AlignRight
                    font.family: "Courier New"
                }
                Label {
                    text: (model.unitPrice * model.quantity).toFixed(2)
                    color: "black"
                    Layout.preferredWidth: 80
                    horizontalAlignment: Text.AlignRight
                    font.family: "Courier New"
                }
                Label {
                    text: model.taxAmount
                    color: "black"
                    Layout.preferredWidth: 80
                    horizontalAlignment: Text.AlignRight
                    font.family: "Courier New"
                }
            }
        }

        // --- TOTALS SECTION ---
        Column {
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            spacing: 2

            Rectangle { width: parent.width; height: 1; color: "black" }
            Rectangle { width: parent.width; height: 1; color: "black" }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: "Tax Amount"; font { pixelSize: 22; bold: true } color: "black"; font.family: "Courier New" }
            Item { Layout.fillWidth: true }
            Label {
                text: _salesModel.totalTaxAmount ? "KES " + _salesModel.totalTaxAmount : "KES " + "0.00"
                font { pixelSize: 22; bold: true } color: "black"; font.family: "Courier New"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: "TOTAL"; font { pixelSize: 22; bold: true } color: "black"; font.family: "Courier New" }
            Item { Layout.fillWidth: true }
            Label {
                text: "KES " + _salesModel.totalAmount
                font { pixelSize: 22; bold: true } color: "black"; font.family: "Courier New"
            }
        }

        // Footer space
        Item { Layout.preferredHeight: 10 }
    }
}

