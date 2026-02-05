import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import POS.UI 1.0

RowLayout {
    width: paymentStack.width
    height: paymentStack.height
    spacing: 20
    Component {
            id: mpesaPaymentView
            MpesaMain { } // Assuming the second file provided is named MpesaView.qml
        }

        Component {
            id: cashPaymentView
            CashPaymentView { } // Assuming the third file provided is named CashProcessingView.qml
        }
    OrderDetailsPanel {
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.75
        // Pass the data model here too
        // _salesModel: _salesModel
    }

    ColumnLayout {
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.25
        spacing: 15



        Label { text: "Select Method"; color: "#95a5a6"; Layout.alignment: Qt.AlignHCenter }

        Button {
            text: "M-PESA"
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            onClicked: {
                salesModel.amount = parseFloat(_salesModel.totalAmount)
                salesModel.startMpesaPayment("");
                paymentStack.replace(mpesaPaymentView)
            }
            background: Rectangle { color: "#1DB954"; radius: 8 }
        }

        Button {
            text: "CASH"
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            onClicked: {
                salesModel.amount = parseFloat(_salesModel.totalAmount);
                salesModel.startCashPayment()
                paymentStack.replace(cashPaymentView)
            }
            background: Rectangle { color: "#f1c40f"; radius: 8 }
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "← BACK"
                flat: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                onClicked: paymentStack.replace(mainPaymentView)
            }

            Button {
                text: " CANCEL"
                flat: true
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                onClicked: {
                    onClicked: {
                        if (paymentStack.depth > 1) {
                            paymentStack.pop(); // Go back to STK Push or Order Summary
                        } else {
                            root.close(); // Close the entire payment dialog [cite: 32]
                        }
                    }
                }
            }
        }


    }
}
