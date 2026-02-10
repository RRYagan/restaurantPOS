import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    property string label: ""
    property real value: 0.0 // 0.0 to 1.0
    property color color: "blue"

    Layout.fillWidth: true
    spacing: 5

    RowLayout {
        Label { text: label; color: "white"; font.pixelSize: 12 }
        Item { Layout.fillWidth: true }
        Label { text: (value * 100).toFixed(0) + "%"; color: "white"; font.bold: true }
    }

    Rectangle {
        Layout.fillWidth: true; height: 8; radius: 4; color: "#1a1a1a"
        Rectangle {
            width: parent.width * value; height: parent.height; radius: 4; color: parent.parent.color
        }
    }
}
