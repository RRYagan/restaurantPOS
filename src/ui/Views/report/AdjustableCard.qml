import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: cardRoot
    property string title: ""
    property string cardIcon: ""
    property string cardValue: ""
    property bool isMinimized: false

    signal minimizeClicked()
    signal onCloseClicked()

    color: window.theme.sidePanelBg
    radius: window.theme.cardRadius
    border.color: window.theme.border
    clip: true

    // Animate height change when minimized
    Behavior on height { NumberAnimation { duration: 250; easing.type: Easing.InOutQuad } }
    height: isMinimized ? 60 : implicitHeight

    ColumnLayout {
        width: parent.width
        spacing: 0

        // CARD HEADER
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            Layout.leftMargin: 15; Layout.rightMargin: 15

            Text { text: cardIcon; color: window.theme.accent; visible: cardIcon !== "" }
            Label { text: title; font.bold: true; color: "white"; Layout.fillWidth: true }

            // Interaction Buttons
            Row {
                spacing: 10
                ToolButton {
                    text: isMinimized ? "□" : "–"
                    onClicked: minimizeClicked()
                }
                ToolButton {
                    text: "×"
                    onClicked: onCloseClicked()
                }
            }
        }

        // KPI CONTENT (Simple view)
        Column {
            Layout.leftMargin: 15; Layout.bottomMargin: 15
            visible: cardValue !== "" && !isMinimized
            Label { text: cardValue; font.pixelSize: 24; font.bold: true; color: "white" }
        }

        // INNER CONTENT LOADER (Reserved for Charts/Tables)
        Item {
            id: contentArea
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height
            visible: !isMinimized
        }
    }
}
