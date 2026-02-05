import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: procContainer
    anchors.fill: parent
    color: "#ffffff"
    radius: 12

    readonly property bool hasActivePayment: salesModel.activePayment !== null
    readonly property bool isCash: salesModel.methodName === "CASH"

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        spacing: 25

        // --- 1. STATUS VISUALS ---
        Item {
            Layout.preferredHeight: 120
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            BusyIndicator {
                anchors.centerIn: parent
                running: salesModel.currentState < 4 // Success/Failed are 4/5
                visible: running
                implicitWidth: 80
                implicitHeight: 80
            }

            Text {
                anchors.centerIn: parent
                text: salesModel.currentStateName === "Success" ? "✅" : "❌"
                font.pixelSize: 80
                visible: salesModel.currentStateName === "Success" || salesModel.currentStateName === "Failed"
            }
        }

        // --- 2. TEXTUAL FEEDBACK ---
        ColumnLayout {
            spacing: 5
            Layout.fillWidth: true

            Label {
                text: isCash ? "CASH PAYMENT" : "M-PESA PAYMENT"
                font.pixelSize: 12
                font.bold: true
                color: "#95a5a6"
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: salesModel.currentMessage
                font.pixelSize: 22
                font.bold: true
                color: salesModel.currentStateName === "Failed" ? "#e74c3c" : "#2c3e50"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Debug Info
            Label {
                text: "DB State: " + salesModel.currentStateName
                font.pixelSize: 10
                color: "#bdc3c7"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // --- 3. TRANSACTION CARD ---
        Rectangle {
            id: dataCard
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 100
            color: "#f8f9fa"
            radius: 10

            // Use methodName to ensure we are in an M-Pesa flow and activePayment exists
            visible: salesModel.methodName === "MPESA" && salesModel.activePayment !== null

            ColumnLayout {
                anchors.centerIn: parent

                Label {
                    text: salesModel.currentStateName === "Success" ? "MPESA RECEIPT" : "CHECKOUT ID (Stored in DB)"
                    font.pixelSize: 10
                    color: "#7f8c8d"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    font.family: "Monospace"
                    font.pixelSize: 14
                    color: "#34495e"
                    Layout.alignment: Qt.AlignHCenter

                    // Final safety check to prevent undefined assignments
                    text: {
                        if (salesModel.methodName !== "MPESA" || !salesModel.activePayment) {
                            return "---";
                        }

                        // At this point, C++ guarantees activePayment is an MpesaPayment object
                        let resp = salesModel.activePayment.lastResponse;
                        if (!resp) return "Processing...";

                        return (salesModel.currentStateName === "Success") ?
                                (resp.receipt || "VERIFIED") :
                                (resp.checkoutId || "---");
                    }
                }
            }
        }

        // --- 4. ACTIONS ---
        ColumnLayout {
            Layout.preferredWidth: parent.width * 0.8
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            Button {
                text: "COMPLETE & PRINT"
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                visible: salesModel.currentStateName === "Success"
                onClicked: {
                    if (globalSalesController.makeOrder()) root.close();
                }
                background: Rectangle { color: "#2ecc71"; radius: 8 }
            }

            Button {
                text: "BACK / RETRY"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                visible: salesModel.currentStateName === "Failed"
                onClicked: {
                    salesModel.preparePayment();
                    paymentStack.pop();
                }
            }
        }
    }
}
