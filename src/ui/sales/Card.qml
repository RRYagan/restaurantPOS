import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control
    property string itemName: ""
    property var price: null // Receives the Money struct/gadget

    background: Rectangle {
        implicitWidth: 160
        implicitHeight: 180
        color: control.down ? "#f0f0f0" : "white"
        radius: 12
        border.color: "#e0e0e0"
    }

    contentItem: ColumnLayout {
        spacing: 10
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 60; height: 60; color: "#ecf0f1"; radius: 30
            Text { anchors.centerIn: parent; text: "🍴"; font.pixelSize: 30 }
        }
        Text {
            text: control.itemName
            font.bold: true; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true
        }
        Text {
            // Updated: Accesses the 'formatted' property of the C++ Money gadget
            text: control.price ? control.price.formatted + " Ksh" : "0.00 Ksh"
            color: "#27ae60"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true
        }
    }
}
