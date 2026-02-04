import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Rectangle {
    id: mpesaPaymentView
    RowLayout {
        width: paymentStack.width
        height: paymentStack.height
        spacing: 20

        // Left 3/4: Dynamic Stackable Content
        StackView {
            id: mpesaInternalStack
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.75

            // Set the initial view to show order details
            initialItem: mpesaSTKPushView

            replaceEnter: Transition { PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
            replaceExit: Transition { PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
        }

        // Right 1/4: Actions
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.25
            spacing: 15

            Label {
                text: "Action Required"
                color: "#95a5a6"
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: "STK PUSH"
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                highlighted: true
                // Switches the internal stack to the STK input view
                onClicked: mpesaInternalStack.replace(mpesaSTKPushView)
            }

            Button {
                text: "QR CODE"
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                // You can add a mpesaQRCodeView component here later mpesaQRView
                // onClicked: console.log("Show QR Code Component")
                onClicked: mpesaInternalStack.replace(mpesaQRView)

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
}
