import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.plasteq.pos.payment 1.0

Rectangle {
    id: procContainer
    anchors.fill: parent
    color: "#ffffff"
    radius: 12

    readonly property bool hasActivePayment: salesModel.activePayment !== null
    readonly property bool isCash: salesModel.methodName === "CASH"

    // Check if we are currently in a "waiting" state (not Success and not Failed)
    // Place this inside your procContainer Rectangle
    readonly property bool isProcessing: {
        const s = salesModel.currentState;
        return s === PaymentStatus.Initiated ||
               s === PaymentStatus.AwaitingAction ||
               s === PaymentStatus.Verifying;
    }
    // Inside procContainer
    onVisibleChanged: {
        if (visible) {
            console.log("PaymentStatus UI opened. Current State: " + salesModel.currentState)
        }
    }
    // --- SUB-COMPONENTS ---
    component MpesaCard: Rectangle {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: 100
        color: "#f8f9fa"
        radius: 10

        ColumnLayout {
            anchors.centerIn: parent
            Label {
                // Header text logic
                text: {
                    if (salesModel.currentState === PaymentStatus.Success) return "MPESA RECEIPT";
                    if (salesModel.currentState === PaymentStatus.Failed) return "PAYMENT FAILED";
                    if (salesModel.currentState === PaymentStatus.Cancelled) return "PAYMENT CANCELLED";
                    return "TRANSACTION STATUS";
                }
                font.pixelSize: 10; color: "#7f8c8d"; Layout.alignment: Qt.AlignHCenter
            }

            BusyIndicator {
                running: isProcessing && !isCash
                visible: running
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                font.family: "Monospace"; font.pixelSize: 14; color: "#34495e"
                Layout.alignment: Qt.AlignHCenter
                // Keep visible if we are successful OR failed
                visible: !isProcessing
                text: {
                    if (salesModel.currentState === PaymentStatus.Success) {
                        return salesModel.activePayment ? salesModel.activePayment.lastResponse.receipt : "VERIFIED";
                    }
                    if (salesModel.currentState === PaymentStatus.Failed) {
                        return salesModel.activePayment ? salesModel.activePayment.lastResponse.receipt : "FAILED";
                    }
                    if (salesModel.currentState === PaymentStatus.Cancelled) {
                        return salesModel.activePayment ? salesModel.activePayment.lastResponse.receipt : "CANCELLED";
                    }
                    return "---";
                }
            }
        }
    }
    component CashCard: Rectangle {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: 100
        color: "#fdfdfd"; border.color: "#ecf0f1"; radius: 10
        ColumnLayout {
            anchors.centerIn: parent
            Label { text: "CASH RECEIVED"; font.pixelSize: 10; color: "#7f8c8d"; Layout.alignment: Qt.AlignHCenter }
            Label {
                text: salesModel.totalAmount
                font.pixelSize: 24; font.bold: true; color: "#27ae60"
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        spacing: 25

        // 1. STATUS VISUALS
        Item {
            Layout.preferredHeight: 120; Layout.fillWidth: true

            BusyIndicator {
                anchors.centerIn: parent
                running: isProcessing // Initiated, Verifying, AwaitingAction
                visible: running
            }

            Text {
                anchors.centerIn: parent
                font.pixelSize: 80
                // Use an explicit check: if it's not processing AND not Idle, show the result
                visible: !isProcessing && salesModel.currentState !== PaymentStatus.Idle
                text: {
                    if (salesModel.currentState === PaymentStatus.Success) return "✅";
                    if (salesModel.currentState === PaymentStatus.Failed) return "❌";
                    if (salesModel.currentState === PaymentStatus.Cancelled) return "⚠️";
                    return "❓"; // Fallback for debugging unknown states
                }
            }
        }
        // 2. TEXTUAL FEEDBACK
        ColumnLayout {
            spacing: 5; Layout.fillWidth: true
            Label {
                text: isCash ? "CASH PAYMENT" : "M-PESA PAYMENT"
                font.pixelSize: 12; font.bold: true; color: "#95a5a6"; Layout.alignment: Qt.AlignHCenter
            }
            Label {
                text: isProcessing ? "Processing Transaction..." : salesModel.currentMessage
                font.pixelSize: 22; font.bold: true
                color: salesModel.currentState === PaymentStatus.Failed ? "#e74c3c" : "#2c3e50"
                horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; Layout.fillWidth: true
            }
        }

        // 3. TRANSACTION CARD
        // Replace your StackLayout block with this:
        StackLayout {
            Layout.fillWidth: true
            // Switch to Cash card if methodName is CASH, otherwise stay on Mpesa/Result
            currentIndex: isCash ? 1 : 0

            MpesaCard {
                // Logic: Show if it's Mpesa OR if we have a result to show (Success/Failed)
                visible: salesModel.methodName === "MPESA" || salesModel.currentState === PaymentStatus.Failed
            }
            CashCard {
                visible: isCash
            }
        }
        // 4. ACTIONS
        ColumnLayout {
            Layout.preferredWidth: parent.width * 0.8; Layout.alignment: Qt.AlignHCenter; spacing: 10

            Button {
                text: "COMPLETE & PRINT"
                Layout.fillWidth: true; Layout.preferredHeight: 60
                visible: salesModel.currentState === PaymentStatus.Success
                onClicked: {
                    // 1. Process the order in DB
                    // if (globalSalesController.makeOrder()) {
                        // 2. Trigger the C++ cleanup we wrote earlier
                        salesModel.clearOrder();
                        root.close();
                    // }
                }
                background: Rectangle { color: "#2ecc71"; radius: 8 }
            }

            // Inside your ACTIONS ColumnLayout (Section 4)
            Button {
                text: "CANCELLED - GO BACK"
                Layout.fillWidth: true; Layout.preferredHeight: 50
                // Show this for Cancelled OR Failed
                visible: salesModel.currentState === PaymentStatus.Failed || salesModel.currentState === PaymentStatus.Cancelled
                onClicked: {
                    salesModel.clearOrder();
                    paymentStack.pop();
                }
            }
        }
    }
}
