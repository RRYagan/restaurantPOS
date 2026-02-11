import QtQuick
import QtQuick.Layouts

Item {
    GridLayout {
        anchors.fill: parent
        anchors.margins: 30
        columns: 2

        // Fix: Use rowSpacing and columnSpacing instead of just 'spacing'
        rowSpacing: 20
        columnSpacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#252525"
            radius: 15

            ColumnLayout {
                anchors.centerIn: parent
                Text { text: "Total Payroll (Feb)"; color: "#888" }
                Text { text: "$42,500.00"; color: "#FFE135"; font.pixelSize: 32; font.bold: true }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#252525"
            radius: 15

            ColumnLayout {
                anchors.centerIn: parent
                Text { text: "Next Pay Date"; color: "#888" }
                Text { text: "Feb 28, 2026"; color: "white"; font.pixelSize: 24; font.bold: true }
            }
        }

        // Module 3: Full-width placeholder
        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1A1A1A"
            border.color: "#333"
            radius: 15
        }
    }
}
