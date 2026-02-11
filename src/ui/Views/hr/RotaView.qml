import QtQuick
import QtQuick.Layouts
import QtQuick.Controls  // <--- Add this line!

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30

        RowLayout {
            Text {
                text: "Weekly Rota"
                color: "white"
                font.pixelSize: 24
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Add Shift"
                // Optional: Style it to match Nano Banana
                contentItem: Text {
                    text: parent.text
                    color: "#000000"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    color: "#FFE135"
                    radius: 8
                }
            }
        }

        GridLayout {
            columns: 7
            Layout.fillWidth: true
            Layout.fillHeight: true
            rowSpacing: 5
            columnSpacing: 5

            Repeater {
                model: 21
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: index % 7 === 0 ? "#FFE135" : "#252525"
                    radius: 6

                    Text {
                        text: index % 7 === 0 ? "SHIFT" : "Off"
                        anchors.centerIn: parent
                        color: index % 7 === 0 ? "black" : "#555"
                        font.pixelSize: 11
                        font.bold: index % 7 === 0
                    }
                }
            }
        }
    }
}
