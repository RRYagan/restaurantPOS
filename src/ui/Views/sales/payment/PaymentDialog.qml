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
    property SalesViewController salesModel
    property var currentUser: "Admin"


    Loader {
        id: processingView
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.75
        sourceComponent: processingComponent

        Component {
            id: processingComponent
            ProcessingView {
            anchors.fill: parent
            }
        }
    }

    Loader {
        id: mainPaymentView
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.75
        sourceComponent: mainPaymentComponent

        Component {
            id: mainPaymentComponent
            MainPaymentView {
            anchors.fill: parent
            }
        }
    }

    Loader {
        id: paymentMethodView
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.75
        sourceComponent: paymentMethodComponent

        Component {
            id: paymentMethodComponent
            PaymentMethodView {
            anchors.fill: parent
            }
        }
    }



    StackView {
        id: paymentStack
        anchors.fill: parent
        clip: true
        initialItem: mainPaymentView

        // Fixed Transitions: Ensure items don't fight for anchors
        replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 } }
        replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
    }


    // --- VIEW 2: Method Selection ---



    // --- VIEW 3: Processing Phase ---
    // Component {
    //     id: processingView

    //     ColumnLayout {
    //         id: processingViewContainer
    //         width: paymentStack.width
    //         height: paymentStack.height
    //         spacing: 20

    //         // Properties for Cash Handling
    //         property double amountReceived: parseFloat(amtInput.text) || 0.0
    //         property double changeDue: amountReceived - root.totalVal

    //         // Map C++ Enums: 0:Initiated, 1:Verifying, 2:AwaitingAction, 3:Success, 4:Failed
    //         property bool isMpesa: salesModel.methodName === "MPESA"

    //         Item { Layout.fillHeight: true }

    //         // --- SECTION 1: DYNAMIC STATUS ICON ---
    //         // Shows a spinner during network calls, or Success/Error icons
    //         ColumnLayout {
    //             Layout.alignment: Qt.AlignHCenter
    //             spacing: 10

    //             BusyIndicator {
    //                 running: salesModel.currentState === 0 || salesModel.currentState === 1
    //                 visible: running
    //                 Layout.alignment: Qt.AlignHCenter
    //             }

    //             Text {
    //                 text: salesModel.currentState === 3 ? "✅" : "❌"
    //                 font.pixelSize: 48
    //                 visible: salesModel.currentState === 3 || salesModel.currentState === 4
    //                 Layout.alignment: Qt.AlignHCenter
    //             }
    //         }

    //         // --- SECTION 2: STATUS MESSAGE ---
    //         Label {
    //             text: salesModel.currentMessage
    //             color: salesModel.currentState === 4 ? "#e74c3c" : "#95a5a6"
    //             font.pixelSize: 18
    //             font.bold: true
    //             Layout.alignment: Qt.AlignHCenter
    //             horizontalAlignment: Text.AlignHCenter
    //             Layout.preferredWidth: parent.width * 0.8
    //             wrapMode: Text.WordWrap
    //         }

    //         // --- SECTION 3: CASH ENTRY (Only visible if Cash & Awaiting Action) ---
    //         ColumnLayout {
    //             Layout.alignment: Qt.AlignHCenter
    //             Layout.preferredWidth: 400
    //             spacing: 15
    //             visible: !isMpesa && salesModel.currentState === 2

    //             Label {
    //                 text: "ENTER CASH RECEIVED:"
    //                 color: "white"
    //                 font.bold: true
    //             }

    //             TextField {
    //                 id: amtInput
    //                 Layout.fillWidth: true
    //                 Layout.preferredHeight: 60
    //                 placeholderText: "0.00"
    //                 font.pixelSize: 24
    //                 color: "white"
    //                 horizontalAlignment: Text.AlignHCenter
    //                 inputMethodHints: Qt.ImhFormattedNumbersOnly
    //                 focus: true
    //                 background: Rectangle {
    //                     color: "#2d0a0a"
    //                     border.color: amtInput.activeFocus ? "#2ecc71" : "#3d1a1a"
    //                     border.width: 2
    //                     radius: 8
    //                 }
    //             }

    //             Rectangle {
    //                 Layout.fillWidth: true
    //                 Layout.preferredHeight: 80
    //                 color: processingViewContainer.changeDue >= 0 ? "#1e3d2a" : "#3d1e1e"
    //                 radius: 8
    //                 visible: processingViewContainer.amountReceived > 0
    //                 ColumnLayout {
    //                     anchors.centerIn: parent
    //                     Label {
    //                         text: processingViewContainer.changeDue >= 0 ? "CHANGE TO GIVE:" : "INSUFFICIENT AMOUNT"
    //                         font.pixelSize: 12
    //                         color: "#95a5a6"
    //                         Layout.alignment: Qt.AlignHCenter
    //                     }
    //                     Label {
    //                         text: "KES " + Math.abs(processingViewContainer.changeDue).toLocaleString(Qt.locale("en_US"), "f", 2)
    //                         font.pixelSize: 24
    //                         font.bold: true
    //                         color: processingViewContainer.changeDue >= 0 ? "#2ecc71" : "#e74c3c"
    //                         Layout.alignment: Qt.AlignHCenter
    //                     }
    //                 }
    //             }
    //         }

    //         // --- SECTION 4: ACTION BUTTONS ---
    //         ColumnLayout {
    //             Layout.alignment: Qt.AlignHCenter
    //             Layout.preferredWidth: 400
    //             spacing: 15

    //             // Confirm Button (Only for Cash)
    //             Button {
    //                 id: confirmBtn
    //                 text: "CONFIRM & PRINT RECEIPT"
    //                 Layout.fillWidth: true
    //                 Layout.preferredHeight: 70
    //                 visible: !isMpesa && salesModel.currentState === 2
    //                 enabled: processingViewContainer.changeDue >= 0
    //                 onClicked: salesModel.confirmAction()
    //                 background: Rectangle {
    //                     color: confirmBtn.enabled ? "#27ae60" : "#34495e"
    //                     radius: 8
    //                 }
    //             }

    //             // Return Button (Visible on Success or Failure)
    //             Button {
    //                 text: salesModel.currentState === 3 ? "COMPLETE ORDER" : "TRY AGAIN"
    //                 Layout.fillWidth: true
    //                 Layout.preferredHeight: 60
    //                 visible: salesModel.currentState === 3 || salesModel.currentState === 4
    //                 onClicked: {
    //                     if (salesModel.currentState === 3) {
    //                         globalSalesController.makeOrder(); // Finalize DB
    //                         dashboardRoot.close(); // Or clear UI
    //                     } else {
    //                         paymentStack.replace(paymentMethodView);
    //                     }
    //                 }
    //             }

    //             // Cancel Button (Always visible unless Success)
    //             Button {
    //                 text: "CANCEL TRANSACTION"
    //                 flat: true
    //                 Layout.alignment: Qt.AlignHCenter
    //                 visible: salesModel.currentState !== 3
    //                 onClicked: {
    //                     salesModel.cancelPayment();
    //                     paymentStack.replace(paymentMethodView);
    //                 }
    //                 contentItem: Text {
    //                     text: parent.text
    //                     color: "#e74c3c"
    //                     font.bold: true
    //                     horizontalAlignment: Text.AlignHCenter
    //                 }
    //             }
    //         }

    //         Item { Layout.fillHeight: true }
    //     }
    // }
}
