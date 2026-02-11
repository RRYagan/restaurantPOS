import QtQuick
import QtQuick.Layouts

Rectangle {
    property string label
    property string value
    property string percent
    property bool isUp
    property string icon

    Layout.fillWidth: true; height: 130
    radius: window.theme.cardRadius
    color: window.theme.sidePanelBg
    border.color: window.theme.border

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 18
        RowLayout {
            Rectangle {
                width: 36; height: 36; radius: 10;
                color: window.theme.surfaceHighlight
                Text { anchors.centerIn: parent; text: icon; color: window.theme.accent; font.pixelSize: 18 }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: percent;
                color: isUp ? window.theme.success : window.theme.danger
                font.bold: true; font.pixelSize: 12
            }
        }
        Text { text: value; color: "white"; font.pixelSize: 24; font.bold: true }
        Text { text: label; color: window.theme.textSecondary; font.pixelSize: 13 }
    }
}
