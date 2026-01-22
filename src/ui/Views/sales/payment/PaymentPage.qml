import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    property SalesView salesModel: null
    property int orderId: -1
    color: "white"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Payment for Order #" + (orderId !== -1 ? orderId : "Current")
            font.pixelSize: 24; font.bold: true
        }

        Text {
            text: "Total Due: " + (salesModel ? salesModel.totalFormatted : "0.00") + " Ksh."
            font.pixelSize: 32; color: "#27ae60"
        }

        Button {
            text: "Confirm Payment"
            onClicked: console.log("Processing payment...")
        }

        Button {
            text: "Cancel"
            flat: true
            onClicked: contentStack.pop()
        }
    }
}
