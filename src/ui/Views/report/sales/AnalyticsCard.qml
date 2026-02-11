import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: cardRoot

    // Custom properties (NO name collisions)
    property string title: ""
    property string value: ""
    property string icon: ""
    property color accentColor: "#3498db"

    Layout.fillWidth: true
    Layout.preferredHeight: 100

    // Rectangle's own color
    color: "#242424"
    radius: 12
    border.color: "#3d3d3d"
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Rectangle {
            width: 40
            height: 40
            radius: 20
            color: "#333333"

            Label {
                anchors.centerIn: parent
                text: cardRoot.icon
                font.pixelSize: 20
            }
        }

        ColumnLayout {
            spacing: 2

            Label {
                text: cardRoot.title
                font.pixelSize: 12
                color: "#95a5a6"
            }

            Label {
                text: cardRoot.value
                font.pixelSize: 16
                font.bold: true
                color: cardRoot.accentColor
            }
        }

        Item { Layout.fillWidth: true }
    }
}
