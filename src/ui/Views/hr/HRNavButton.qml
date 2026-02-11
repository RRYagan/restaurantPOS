import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control
    Layout.fillWidth: true
    contentItem: Text {
        text: control.text
        color: control.hovered ? "#000000" : "#FFFFFF"
        font.bold: true
        horizontalAlignment: Text.AlignLeft
        leftPadding: 40
    }

    background: Rectangle {
        implicitHeight: 45
        color: control.hovered ? "#FFE135" : "transparent"
        radius: 8
        Behavior on color { ColorAnimation { duration: 200 } }
    }
}
