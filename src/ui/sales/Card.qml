import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control
    property string itemName: ""
    property var priceCents: null // Use var for the Money gadget
    property string iconSource: ""

    background: Rectangle {
        implicitWidth: 160
        implicitHeight: 180
        color: control.down ? "#f0f0f0" : "white"
        radius: 12
        border.color: control.visualFocus ? "#3498db" : "#e0e0e0"
        border.width: 2
    }

    contentItem: ColumnLayout {
        spacing: 10
        Image {
            source: control.iconSource || "default.png"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            fillMode: Image.PreserveAspectFit
        }

        Text {
            text: control.itemName
            font.pixelSize: 16
            font.bold: true
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        Text {
            // Using the .formatted property from your Money.h gadget
            text: control.priceCents ? control.priceCents.formatted + "Ksh." : "0.00 Ksh."
            color: "#27ae60"
            font.pixelSize: 14
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
