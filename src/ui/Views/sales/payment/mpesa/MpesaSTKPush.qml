import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import POS.UI 1.0

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

        Text {
            text: "Enter M-Pesa Number"
            color: "black"
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: phoneField
            placeholderText: "2547XXXXXXXX"
            color: "black"
            focus: true
            horizontalAlignment: TextInput.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width * 0.6
            background: Rectangle {
                implicitHeight: 50
                color: "#ffffff"
                border.color: "#95a5a6"
                radius: 4
            }
        }

        Button {
            text: "SEND STK PUSH"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 50
            Layout.preferredWidth: parent.width * 0.6
            onClicked: {
                // salesModel.preparePayment();
                // salesModel.setAmount(totalVal);
                // salesModel.startMpesaPayment(phoneField.text);
                salesModel.startMpesaPayment("254727027979");
                // Push processingView to the nested stack instead of the global one
                paymentStack.push(processingView);
            }
        }

        Item { Layout.fillHeight: true } // Spacer
    }

}


// MPESA QR
