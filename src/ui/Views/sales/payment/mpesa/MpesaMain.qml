import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

RowLayout {
    id: root
    anchors.fill: parent // Ensure the RowLayout has a size!
    spacing: 20

    // --- 1. THE VIEW BLUEPRINTS ---
    Component {
        id: mpesaSTKPushView
        MpesaSTKPush { anchors.fill: parent }
    }

    Component {
        id: mpesaQRView
        MpesaQR { anchors.fill: parent }
    }

    // --- 2. LEFT 3/4: THE DYNAMIC AREA ---
    StackView {
        id: mpesaInternalStack
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.75

        initialItem: mpesaSTKPushView // Starts with STK Push

        replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
        replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
    }

    // --- 3. RIGHT 1/4: THE BUTTONS ---
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
            onClicked: mpesaInternalStack.replace(mpesaSTKPushView)
        }

        Button {
            text: "QR CODE"
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            onClicked: mpesaInternalStack.replace(mpesaQRView)
        }

        Item { Layout.fillHeight: true } // Spacer

        // Bottom Navigation Buttons
        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "← BACK"
                onClicked: paymentStack.pop()
            }
            Button {
                text: "CANCEL"
                onClicked: root.close() // Ensure root or parent has a close() method
            }
        }
    }
}
