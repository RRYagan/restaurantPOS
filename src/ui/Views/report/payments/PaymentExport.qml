import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30

        Rectangle {
            width: 500; height: 300
            color: window.theme.surface
            border.color: window.theme.border; radius: 15

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 20

                Label {
                    text: "Export Financial Report"
                    font.pixelSize: 22; font.bold: true; color: "white"
                }

                RowLayout {
                    spacing: 15
                    Button { text: "CSV"; Layout.preferredWidth: 100 }
                    Button { text: "PDF"; Layout.preferredWidth: 100 }
                    Button { text: "Excel"; Layout.preferredWidth: 100 }
                }

                TextField {
                    placeholderText: "Recipient Email..."
                    Layout.preferredWidth: 330
                }

                Button {
                    text: "Send Report via Email"
                    Layout.fillWidth: true
                    highlighted: true
                }
            }
        }
    }
}
