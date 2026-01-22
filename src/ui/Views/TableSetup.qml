import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.centerIn: parent
        Text { text: "Table Configuration"; font.pixelSize: 24; font.bold: true }
        Text { text: "Manage restaurant floor plan and table numbers here." }
        Button { text: "Add New Table"; highlighted: true }
    }
}
