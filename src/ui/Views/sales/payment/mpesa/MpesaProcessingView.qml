import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Rectangle {
    id: procContainer
    width: paymentStack.width
    height: paymentStack.height
    color: "#ffffff"
    radius: 12

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.85
        spacing: 25

        // --- 1. VISUAL STATUS INDICATOR ---
        Item {
            Layout.preferredHeight: 100
            Layout.alignment: Qt.AlignHCenter

            // Loading Spinner (States: Initiated, Verifying, Awaiting PIN)
            BusyIndicator {
                anchors.centerIn: parent
                running: paymentCtrl.currentState < 3
                visible: running
                implicitWidth: 70
                implicitHeight: 70
            }

            // Success Icon (State: Success)
            Text {
                anchors.centerIn: parent
                text: "✅"
                font.pixelSize: 70
                visible: paymentCtrl.currentState === 3
            }

            // Error Icon (State: Failed / Cancelled)
            Text {
                anchors.centerIn: parent
                text: "❌"
                font.pixelSize: 70
                visible: paymentCtrl.currentState === 4
            }
        }

        // --- 2. MESSAGE DISPLAY ---
        ColumnLayout {
            spacing: 8
            Layout.fillWidth: true

            Label {
                text: {
                    if (paymentCtrl.currentState === 3) return "TRANSACTION SUCCESSFUL"
                    if (paymentCtrl.currentState === 4) return "TRANSACTION FAILED"
                    return "MPESA EXPRESS"
                }
                font.pixelSize: 12
                font.bold: true
                color: "#bdc3c7"
                Layout.alignment: Qt.AlignHCenter
                // letterSpacing: 1.5
            }

            Label {
                id: statusLabel
                text: paymentCtrl.currentMessage // Bound to C++ emit messageUpdated
                font.pixelSize: 20
                font.bold: true
                color: paymentCtrl.currentState === 4 ? "#e74c3c" : "#2c3e50"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }
        }

        // --- 3. ACTION BUTTONS ---
        ColumnLayout {
            Layout.preferredWidth: parent.width * 0.8
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            // Visible only on Success
            Button {
                text: "PRINT RECEIPT & FINISH"
                Layout.fillWidth: true
                Layout.preferredHeight: 55
                visible: paymentCtrl.currentState === 3
                onClicked: {
                    if (globalSalesController.makeOrder()) {
                        dashboardRoot.close();
                    }
                }
                background: Rectangle { color: "#2ecc71"; radius: 6 }
                contentItem: Text { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }

            // Visible only on Failure
            Button {
                text: "TRY AGAIN"
                Layout.fillWidth: true
                Layout.preferredHeight: 55
                visible: paymentCtrl.currentState === 4
                onClicked: paymentStack.pop() // Go back to phone input
                background: Rectangle { color: "#34495e"; radius: 6 }
                contentItem: Text { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }

            // Cancel button visible while processing
            Button {
                text: "CANCEL"
                flat: true
                Layout.alignment: Qt.AlignHCenter
                visible: paymentCtrl.currentState < 3
                onClicked: {
                    paymentCtrl.cancelPayment();
                    paymentStack.pop();
                }
            }
        }
    }
}
