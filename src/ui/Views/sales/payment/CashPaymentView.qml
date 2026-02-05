import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: cashPaymentRoot
    anchors.fill: parent
    color: "#1a0505" // Dark theme to match the background

    // Logic for calculating change based on the order total
    property double amountReceived: parseFloat(amtInput.text) || 0.0
    property double changeDue: amountReceived - salesModel.totalAmount

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 20

        Item { Layout.fillHeight: true }

        // --- CASH ENTRY SECTION ---
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 400
            spacing: 15

            Label {
                text: "ENTER CASH RECEIVED:"
                color: "white"
                font.bold: true
                font.pixelSize: 14
            }

            TextField {
                id: amtInput
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                placeholderText: "0.00"
                font.pixelSize: 32
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                focus: true

                background: Rectangle {
                    color: "#2d0a0a"
                    border.color: amtInput.activeFocus ? "#2ecc71" : "#3d1a1a"
                    border.width: 2
                    radius: 12
                }
                Component.onCompleted: forceActiveFocus()
            }

            // Change feedback box
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                color: cashPaymentRoot.changeDue >= 0 ? "#1e3d2a" : "#3d1e1e"
                radius: 12
                visible: cashPaymentRoot.amountReceived > 0

                ColumnLayout {
                    anchors.centerIn: parent
                    Label {
                        text: cashPaymentRoot.changeDue >= 0 ? "CHANGE TO GIVE:" : "INSUFFICIENT AMOUNT"
                        font.pixelSize: 14
                        color: "#95a5a6"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Label {
                        text: "KES " + Math.abs(cashPaymentRoot.changeDue).toLocaleString(Qt.locale("en_US"), "f", 2)
                        font.pixelSize: 32
                        font.bold: true
                        color: cashPaymentRoot.changeDue >= 0 ? "#2ecc71" : "#e74c3c"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        // --- CONFIRMATION ACTION ---
        Button {
            id: confirmBtn
            text: "CONFIRM PAYMENT"
            Layout.preferredWidth: 400
            Layout.preferredHeight: 70
            Layout.alignment: Qt.AlignHCenter
            enabled: cashPaymentRoot.changeDue >= 0 && cashPaymentRoot.amountReceived > 0

            onClicked: {
                salesModel.confirmAction();
                // We transition to the unified ProcessingView for the final success screen
                paymentStack.replace(processingView);
            }

            background: Rectangle {
                color: confirmBtn.enabled ? "#2ecc71" : "#2c3e50"
                radius: 12
            }
        }

        Button {
            text: "CANCEL"
            flat: true
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                salesModel.cleanUpActivePayment();
                paymentStack.pop();
            }
        }

        Item { Layout.fillHeight: true }
    }
}
