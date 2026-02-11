import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 30

        Text { text: "Hiring Pipeline"; color: "#FFE135"; font.pixelSize: 28; font.bold: true }

        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true
            model: ["Chef de Partie", "Head Server", "Mixologist"]
            spacing: 10
            delegate: Rectangle {
                width: parent.width; height: 80; color: "#252525"; radius: 10
                RowLayout {
                    anchors.fill: parent; anchors.margins: 20
                    ColumnLayout {
                        Text { text: modelData; color: "white"; font.bold: true; font.pixelSize: 16 }
                        Text { text: "3 Applicants Pending"; color: "#888" }
                    }
                    Item { Layout.fillWidth: true }
                    Button { text: "View Applications"; flat: true; contentItem: Text { text: "View"; color: "#FFE135" } }
                }
            }
        }
    }
}
