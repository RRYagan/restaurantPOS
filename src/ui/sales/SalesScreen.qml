import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: root
    color: "#f8f9fa"
    property SalesModel salesModel: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Text {
            text: "Current Order"
            font.pixelSize: 24
            font.bold: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: salesModel
            delegate: ItemDelegate {
                width: parent.width
                // Uses the .value property of the Money gadget for numeric formatting
                text: model.quantity + "x " + model.name + " - " +
                      (model.price ? model.price.formatted : "0.00") + " Ksh."
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "Add Test Item"
                highlighted: true
                onClicked: salesModel.addItem(101)
            }
            Item { Layout.fillWidth: true }
            Text {
                font.pixelSize: 20
                font.bold: true
                // Assuming salesModel provides a formatted total string or gadget
                text: "Total: " + salesModel.totalFormatted + " Ksh."
            }
        }


    }
}
