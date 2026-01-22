import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: control
    title: "CUSTOMIZE ORDER"
    modal: true
    anchors.centerIn: Overlay.overlay // Aligning with PaymentDialog [cite: 31]

    // Size based on parent, similar to PaymentDialog [cite: 31]
    width: parent.width * 0.7
    height: parent.height * 0.7

    // --- Matches PaymentDialog Background --- [cite: 31]
    background: Rectangle {
        color: "#1a0505"
        border.color: "#3d1a1a"
        border.width: 2
        radius: 12
    }

    header: Item {
        height: 70
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 5
            Label {
                text: control.title
                color: "#e74c3c" // Danger/Accent color from PaymentDialog [cite: 38]
                font.bold: true
                font.pixelSize: 22
                Layout.alignment: Qt.AlignHCenter
            }
            Rectangle {
                height: 1;
                color: "#3d1a1a";
                Layout.preferredWidth: control.width * 0.8
            } // Separator line [cite: 39]
        }
    }

    property var availableModifiers: []
    property var selectedModifiers: []
    signal modifiersSelected(var selectedList)

    contentItem: ColumnLayout {
        spacing: 20
        anchors.margins: 20

        Label {
            text: "SELECT ADD-ONS"
            font.pixelSize: 14
            font.bold: true
            color: "#95a5a6" // Secondary text color [cite: 41, 62]
            Layout.alignment: Qt.AlignLeft
        }

        ListView {
            id: modifierList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: availableModifiers
            spacing: 10

            delegate: CheckDelegate {
                id: delegate
                width: modifierList.width
                height: 60

                background: Rectangle {
                    // Dark surface color with highlight on selection [cite: 85]
                    color: delegate.checked ? "#250a0a" : "transparent"
                    radius: 8
                    border.color: delegate.checked ? "#2ecc71" : "#3d1a1a"
                    border.width: delegate.checked ? 2 : 1
                }

                contentItem: RowLayout {
                    spacing: 15
                    // Check Indicator Customization
                    Rectangle {
                        width: 24; height: 24
                        radius: 4
                        color: delegate.checked ? "#2ecc71" : "#2d0a0a"
                        border.color: "#3d1a1a"
                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            visible: delegate.checked
                            color: "white"
                        }
                    }

                    Text {
                        text: modelData.name
                        font.pixelSize: 18
                        color: "white" // Main text [cite: 42]
                        Layout.fillWidth: true
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        text: modelData.price_cents > 0 ?
                              "+ KES " + (modelData.price_cents / 100).toFixed(2) : "FREE"
                        color: "#2ecc71" // Success/Price color [cite: 58, 90]
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                    }
                }

                onCheckedChanged: {
                    if (checked) {
                        selectedModifiers.push(modelData)
                    } else {
                        selectedModifiers = selectedModifiers.filter(m => m.name !== modelData.name)
                    }
                }
            }
        }
    }

    footer: RowLayout {
        spacing: 15
        Layout.margins: 20

        Button {
            text: "CANCEL"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            flat: true
            onClicked: control.reject()
            contentItem: Text {
                text: parent.text
                color: "#e74c3c" // Danger color [cite: 98]
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Button {
            text: "APPLY CHANGES"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            highlighted: true
            onClicked: control.accept()

            background: Rectangle {
                color: "#27ae60" // Success green [cite: 95]
                radius: 8
            }
        }
    }

    onAccepted: {
        modifiersSelected(selectedModifiers)
        selectedModifiers = []
    }
}
