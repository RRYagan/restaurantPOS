import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import POS.UI 1.0

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
