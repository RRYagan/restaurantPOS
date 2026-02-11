import QtQuick
import QtQuick.Controls
import QtQuick.Layouts  // <--- This was missing!

Item {
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 30
        Text { text: "Leave Requests"; color: "white"; font.pixelSize: 24 }

        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; color: "#252525"; radius: 12
            ListView {
                anchors.fill: parent; anchors.margins: 10
                model: ListModel {
                    ListElement { name: "John Doe"; type: "Sick Leave"; status: "Pending" }
                    ListElement { name: "Jane Smith"; type: "Vacation"; status: "Approved" }
                }
                delegate: RowLayout {
                    width: parent.width; height: 50
                    Text { text: name; color: "white"; Layout.preferredWidth: 150 }
                    Text { text: type; color: "#888"; Layout.preferredWidth: 150 }
                    Rectangle {
                        width: 80; height: 24; radius: 12; color: status === "Approved" ? "#2ecc71" : "#FFE135"
                        Text { text: status; anchors.centerIn: parent; color: "black"; font.pixelSize: 11; font.bold: true }
                    }
                }
            }
        }
    }
}
