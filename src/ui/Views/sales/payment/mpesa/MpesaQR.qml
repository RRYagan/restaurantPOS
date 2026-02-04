import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects


Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#fffdf0"
    border.color: "#e0e0e0"
    radius: 8

    ColumnLayout {
        spacing: 15
        anchors.fill: parent
        anchors.margins: 20
        // spacing: 20


        Item { Layout.fillHeight: true } // Spacer

        // Text {
        //     text: "Enter M-Pesa Number"
        //     color: "white"
        //     font.pixelSize: 18
        //     Layout.alignment: Qt.AlignHCenter
        // }

        TextField {
            id: qrField
            placeholderText: "QR Code"
            color: "white"
            focus: true
            horizontalAlignment: TextInput.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width
            background: Rectangle {
                implicitHeight: 200
                color: "#2d1a1a"
                border.color: "#95a5a6"
                radius: 4
            }
        }

        Button {
            text: "PROCESS"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 50
            Layout.preferredWidth: parent.width * 0.6
            onClicked: {
                // Push processingView to the nested stack instead of the global one
                paymentStack.replace(processingView)
            }
        }

        Item { Layout.fillHeight: true } // Spacer
    }
}

