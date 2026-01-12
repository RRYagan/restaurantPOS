import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Dialog {
    id: root
    modal: true
    anchors.centerIn: Overlay.overlay

    // Theme and Size
    width: parent.width * 0.95
    height: parent.height * 0.95

    background: Rectangle {
        color: "#1a0505"
        border.color: "#3d1a1a"
        border.width: 2
        radius: 12
    }

    // property MenuViewController salesModel
    property PaymentController paymentCtrl
    property var currentUser: "Admin"
    // Place these at the top of PaymentDialog.qml
    property double totalVal: {
        if (!salesModel || !salesModel.totalFormatted) return 0.0;
        // Remove commas and currency symbols if present
        let clean = salesModel.totalFormatted.replace(/[^0-9.]/g, '');
        return parseFloat(clean) || 0.0;
    }

    property double subTotal: totalVal / 1.16
    property double taxVal: totalVal - subTotal

    StackView {
        id: paymentStack
        anchors.fill: parent
        clip: true
        initialItem: successView

        // Fixed Transitions: Ensure items don't fight for anchors
        replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 } }
        replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
    }

    // --- SHARED COMPONENT: Order Details Summary (3/4 Width) ---
    Component {
        id: orderDetailsPanel
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#250a0a"
            radius: 8
            border.color: "#3d1a1a"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                Label {
                    text: "ORDER SUMMARY"
                    color: "#e74c3c"
                    font.bold: true
                    font.pixelSize: 22
                }

                Rectangle { height: 1; color: "#3d1a1a"; Layout.fillWidth: true }

                GridLayout {
                    columns: 2
                    rowSpacing: 15
                    columnSpacing: 50

                    Label { text: "Order ID:"; color: "#95a5a6"; font.pixelSize: 16 }
                    Label { text: salesModel.currentOrderId; color: "white"; font.bold: true; font.pixelSize: 16 }

                    Label { text: "Server:"; color: "#95a5a6"; font.pixelSize: 16 }
                    Label { text: currentUser; color: "white"; font.bold: true; font.pixelSize: 16 }

                    Label { text: "Items Count:"; color: "#95a5a6"; font.pixelSize: 16 }
                    Label { text: salesModel.proxy ? salesModel.proxy.rowCount() : "0"; color: "white"; font.pixelSize: 16 }
                }

                Item { Layout.fillHeight: true } // Spacer

                Rectangle { height: 1; color: "#3d1a1a"; Layout.fillWidth: true }

                // Sub-Total Row
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "Sub-Total"; color: "#95a5a6"; font.pixelSize: 16 }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: "KES " + root.subTotal.toLocaleString(Qt.locale("en_US"), "f", 2)
                        color: "white"; font.pixelSize: 16
                    }
                }

                // Tax Row
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "VAT (16%)"; color: "#95a5a6"; font.pixelSize: 16 }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: "KES " + root.taxVal.toLocaleString(Qt.locale("en_US"), "f", 2)
                        color: "white"; font.pixelSize: 16
                    }
                }

                Rectangle { height: 1; color: "#3d1a1a"; Layout.fillWidth: true; Layout.topMargin: 5; Layout.bottomMargin: 5 }

                // Final Total Row
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "TOTAL "; color: "white"; font.pixelSize: 26; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: salesModel.totalFormatted;
                        color: "#2ecc71";
                        font.pixelSize: 32;
                        font.bold: true
                    }
                }
            }
        }
    }

    // --- VIEW 1: Success Phase ---
    Component {
        id: successView
        RowLayout {
            width: paymentStack.width
            height: paymentStack.height
            spacing: 20

            // Left 3/4: Details
            Loader {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.75
                sourceComponent: orderDetailsPanel
            }

            // Right 1/4: Actions
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.25
                spacing: 15

                Label { text: "Action Required"; color: "#95a5a6"; Layout.alignment: Qt.AlignHCenter }

                Button {
                    text: "PAY NOW"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    highlighted: true
                    onClicked: paymentStack.replace(methodView)
                }

                Button {
                    text: "PAY LATER"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    onClicked: { salesModel.clearOrder(); root.close(); }
                }

                Item { Layout.fillHeight: true }
                RowLayout {
                    Layout.fillWidth: true
                    // Button {
                    //     text: "← BACK"
                    //     flat: true
                    //     onClicked: paymentStack.replace(successView)
                    // }

                    Button {
                        text: "CANCEL"
                        flat: true
                        Layout.alignment: Qt.AlignRight| Qt.AlignBottom

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

    // --- VIEW 2: Method Selection ---
    Component {
        id: methodView
        RowLayout {
            width: paymentStack.width
            height: paymentStack.height
            spacing: 20

            Loader {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.75
                sourceComponent: orderDetailsPanel
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
                        paymentCtrl.amount = parseFloat(salesModel.totalFormatted.replace(/,/g, ''))
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
                        paymentCtrl.amount = parseFloat(salesModel.totalFormatted.replace(/,/g, ''))
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
                        onClicked: paymentStack.replace(successView)
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
                        onClicked: paymentStack.replace(successView)
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
    // --- VIEW 2: M-PESA PHONE INPUT ---

    Component {
        id: mpesaSTKPushView
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#250a0a"
            radius: 8
            border.color: "#3d1a1a"
            ColumnLayout {
                spacing: 15
                anchors.fill: parent
                anchors.margins: 20
                // spacing: 20


                Item { Layout.fillHeight: true } // Spacer

                Text {
                    text: "Enter M-Pesa Number"
                    color: "white"
                    font.pixelSize: 18
                    Layout.alignment: Qt.AlignHCenter
                }

                TextField {
                    id: phoneField
                    placeholderText: "2547XXXXXXXX"
                    color: "white"
                    focus: true
                    horizontalAlignment: TextInput.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width * 0.6
                    background: Rectangle {
                        implicitHeight: 50
                        color: "#2d1a1a"
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
                        // paymentCtrl.setAmount(totalVal);
                        paymentCtrl.startMpesaPayment(phoneField.text);

                        // Push processingView to the nested stack instead of the global one
                        mpesaInternalStack.push(processingView);
                    }
                }

                Item { Layout.fillHeight: true } // Spacer
            }
        }
    }

    // MPESA QR
    Component {
        id: mpesaQRView
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#250a0a"
            radius: 8
            border.color: "#3d1a1a"
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
    }

    // --- VIEW 3: Processing Phase ---
    Component {
        id: processingView

        // Use a top-level ColumnLayout with a unique ID
        ColumnLayout {
            id: cashViewContainer
            width: paymentStack.width
            height: paymentStack.height
            spacing: 20

            // Define properties here so they are accessible by ID anywhere in this component
            property double amountReceived: parseFloat(amtInput.text) || 0.0
            property double changeDue: amountReceived - root.totalVal

            Item { Layout.fillHeight: true }

            Label {
                text: paymentCtrl.currentMessage
                color: "#95a5a6"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
            }

            // Cash Entry Section
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 400
                spacing: 15
                visible: paymentCtrl.currentState === 2

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

                // Change Display - Using 'cashViewContainer' ID instead of parent.parent
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    // Reference by ID
                    color: cashViewContainer.changeDue >= 0 ? "#1e3d2a" : "#3d1e1e"
                    radius: 8
                    visible: cashViewContainer.amountReceived > 0

                    ColumnLayout {
                        anchors.centerIn: parent
                        Label {
                            text: cashViewContainer.changeDue >= 0 ? "CHANGE TO GIVE:" : "INSUFFICIENT AMOUNT"
                            font.pixelSize: 12
                            color: "#95a5a6"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Label {
                            // Reference by ID
                            text: "KES " + Math.abs(cashViewContainer.changeDue).toLocaleString(Qt.locale("en_US"), "f", 2)
                            font.pixelSize: 24
                            font.bold: true
                            color: cashViewContainer.changeDue >= 0 ? "#2ecc71" : "#e74c3c"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 400
                spacing: 15

                Button {
                    id: confirmBtn
                    text: "CONFIRM & PRINT RECEIPT"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    highlighted: true

                    visible: paymentCtrl.currentState === 2
                    // Reference by ID
                    enabled: cashViewContainer.changeDue >= 0

                    onClicked: paymentCtrl.confirmAction()

                    background: Rectangle {
                        color: confirmBtn.enabled ? "#27ae60" : "#34495e"
                        radius: 8
                    }
                }

                Button {
                    text: "CANCEL TRANSACTION"
                    flat: true
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: {
                        paymentCtrl.cancelPayment();
                        paymentStack.replace(methodView);
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
