import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.centerIn: parent
        Text { text: "Appearance Settings"; font.pixelSize: 24; font.bold: true }
        RowLayout {
            Button { text: "Light Mode" }
            Button { text: "Dark Mode" }
        }
    }
}
