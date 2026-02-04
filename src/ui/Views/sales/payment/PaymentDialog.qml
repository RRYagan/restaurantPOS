import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import POS.UI 1.0

Dialog {
    id: root
    modal: true
    anchors.centerIn: Overlay.overlay

    // Theme and Size
    width: parent.width
    height: parent.height
    // --- THEME PROPERTIES ---
    // property bool isDarkMode: true
    // property color themeBg: isDarkMode ? "#1a0505" : "#ffffff"
    // property color themeFg: isDarkMode ? "#ffffff" : "#1a0505"
    // property color themeBorder: isDarkMode ? "#3d1a1a" : "#e0e0e0"
    // property color themeInputBg: isDarkMode ? "#2d1a1a" : "#f5f5f5"



    background: Rectangle {
        color: "#ffffff"
        border.color: "#e0e0e0"
        border.width: 2
        radius: 12
    }
    // property MenuViewController _salesModel
    property PaymentController paymentCtrl
    property var currentUser: "Admin"

    StackView {
        id: paymentStack
        anchors.fill: parent
        clip: true
        initialItem: mainPaymentView

        // Fixed Transitions: Ensure items don't fight for anchors
        replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 } }
        replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
    }

    Component {
        id: mainPaymentView
        Rectangle {
            width: paymentStack.width
            height: paymentStack.height
            color: "#f4f4f4" // Lighter, neutral background for the whole view

            RowLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 30

                // Left Side: The Receipt
                OrderDetailsPanel {
                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.width * 0.65
                    // Pass the data model from root to the component
                    // _salesModel: root._salesModel
                }

                // Right Side: Action Buttons
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.width * 0.35
                    spacing: 20

                    Label {
                        text: "Payment Pending"
                        color: "#2c3e50"
                        font.pixelSize: 22
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    // Bright "Success" Green Button
                    Button {
                        id: payNowBtn
                        text: "PAY NOW"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 80
                        font.bold: true
                        onClicked: paymentStack.replace(paymentMethodView)

                        background: Rectangle {
                            color: payNowBtn.down ? "#219150" : "#2ecc71"
                            radius: 12
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowColor: "#40000000"
                                shadowBlur: 8
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 18
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    // Bright "Warning" Amber Button
                    Button {
                        id: payLaterBtn
                        text: "PAY LATER"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 80
                        onClicked: { _salesModel.clearOrder(); root.close(); }

                        background: Rectangle {
                            color: payLaterBtn.down ? "#d35400" : "#e67e22"
                            radius: 12
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 18
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Item { Layout.fillHeight: true }

                    // High Visibility Cancel
                    Button {
                        text: "CANCEL ORDER"
                        flat: true
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: root.close()
                        contentItem: Text {
                            text: parent.text
                            color: "#e74c3c"
                            font.bold: true
                            font.pixelSize: 16
                        }
                    }
                }
            }
        }
    }
    // --- VIEW 2: Method Selection ---
    Component {
        id: paymentMethodView
        RowLayout {
            width: paymentStack.width
            height: paymentStack.height
            spacing: 20

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
                        paymentCtrl.amount = parseFloat(_salesModel.totalAmount)
                        paymentCtrl.startMpesaPayment("");
                        paymentStack.replace(mpesaPaymentView)
                    }
                    background: Rectangle { color: "#1DB954"; radius: 8 }
                }

                Button {
                    text: "CASH"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    onClicked: {
                        paymentCtrl.amount = parseFloat(_salesModel.totalAmount);
                        paymentCtrl.startCashPayment()
                        paymentStack.replace(processingView)
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
    }

    Component {
        id: mpesaPaymentView
        MpesaMain {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.75

        }
    }
    // --- VIEW 2: M-PESA PHONE INPUT ---

    Component {
        id: mpesaSTKPushView
        MpesaSTKPush {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.75

        }
    }

    // MPESA QR
    Component {
        id: mpesaQRView
        MpesaQR {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.75

        }
    }

    // --- VIEW 3: Processing Phase ---
    Component {
        id: processingView

        ColumnLayout {
            id: processingViewContainer
            width: paymentStack.width
            height: paymentStack.height
            spacing: 20

            // Properties for Cash Handling
            property double amountReceived: parseFloat(amtInput.text) || 0.0
            property double changeDue: amountReceived - root.totalVal

            // Map C++ Enums: 0:Initiated, 1:Verifying, 2:AwaitingAction, 3:Success, 4:Failed
            property bool isMpesa: paymentCtrl.methodName === "MPESA"

            Item { Layout.fillHeight: true }

            // --- SECTION 1: DYNAMIC STATUS ICON ---
            // Shows a spinner during network calls, or Success/Error icons
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                BusyIndicator {
                    running: paymentCtrl.currentState === 0 || paymentCtrl.currentState === 1
                    visible: running
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: paymentCtrl.currentState === 3 ? "✅" : "❌"
                    font.pixelSize: 48
                    visible: paymentCtrl.currentState === 3 || paymentCtrl.currentState === 4
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // --- SECTION 2: STATUS MESSAGE ---
            Label {
                text: paymentCtrl.currentMessage
                color: paymentCtrl.currentState === 4 ? "#e74c3c" : "#95a5a6"
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.preferredWidth: parent.width * 0.8
                wrapMode: Text.WordWrap
            }

            // --- SECTION 3: CASH ENTRY (Only visible if Cash & Awaiting Action) ---
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 400
                spacing: 15
                visible: !isMpesa && paymentCtrl.currentState === 2

                Label {
                    text: "ENTER CASH RECEIVED:"
                    color: "white"
                    font.bold: true
                }

                TextField {
                    id: amtInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    placeholderText: "0.00"
                    font.pixelSize: 24
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    focus: true
                    background: Rectangle {
                        color: "#2d0a0a"
                        border.color: amtInput.activeFocus ? "#2ecc71" : "#3d1a1a"
                        border.width: 2
                        radius: 8
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    color: processingViewContainer.changeDue >= 0 ? "#1e3d2a" : "#3d1e1e"
                    radius: 8
                    visible: processingViewContainer.amountReceived > 0
                    ColumnLayout {
                        anchors.centerIn: parent
                        Label {
                            text: processingViewContainer.changeDue >= 0 ? "CHANGE TO GIVE:" : "INSUFFICIENT AMOUNT"
                            font.pixelSize: 12
                            color: "#95a5a6"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Label {
                            text: "KES " + Math.abs(processingViewContainer.changeDue).toLocaleString(Qt.locale("en_US"), "f", 2)
                            font.pixelSize: 24
                            font.bold: true
                            color: processingViewContainer.changeDue >= 0 ? "#2ecc71" : "#e74c3c"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // --- SECTION 4: ACTION BUTTONS ---
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 400
                spacing: 15

                // Confirm Button (Only for Cash)
                Button {
                    id: confirmBtn
                    text: "CONFIRM & PRINT RECEIPT"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    visible: !isMpesa && paymentCtrl.currentState === 2
                    enabled: processingViewContainer.changeDue >= 0
                    onClicked: paymentCtrl.confirmAction()
                    background: Rectangle {
                        color: confirmBtn.enabled ? "#27ae60" : "#34495e"
                        radius: 8
                    }
                }

                // Return Button (Visible on Success or Failure)
                Button {
                    text: paymentCtrl.currentState === 3 ? "COMPLETE ORDER" : "TRY AGAIN"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    visible: paymentCtrl.currentState === 3 || paymentCtrl.currentState === 4
                    onClicked: {
                        if (paymentCtrl.currentState === 3) {
                            globalSalesController.makeOrder(); // Finalize DB
                            dashboardRoot.close(); // Or clear UI
                        } else {
                            paymentStack.replace(paymentMethodView);
                        }
                    }
                }

                // Cancel Button (Always visible unless Success)
                Button {
                    text: "CANCEL TRANSACTION"
                    flat: true
                    Layout.alignment: Qt.AlignHCenter
                    visible: paymentCtrl.currentState !== 3
                    onClicked: {
                        paymentCtrl.cancelPayment();
                        paymentStack.replace(paymentMethodView);
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#e74c3c"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }}
