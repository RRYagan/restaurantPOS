import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: detailsRoot
    color: "transparent"
    property var saleData // Passed from the push()
    signal back()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header with Back Button
        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "⬅ Back to List"
                onClicked: detailsRoot.back()
                background: Rectangle { color: theme.surfaceHighlight; radius: 4 }
            }
            Label {
                text: "Transaction Details: #" + saleData.orderId
                font.pixelSize: 18; font.bold: true; color: theme.textMain
            }
            Item { Layout.fillWidth: true }
            Label {
                text: saleData.displayDate
                color: theme.textSecondary
            }
        }

        RowLayout {
            spacing: 20
            Layout.fillHeight: true


            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: theme.surface
                radius: 8
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Header for Items
                    Rectangle {
                        Layout.fillWidth: true
                        height: 35
                        color: theme.surfaceHighlight
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
                            Label { text: "Item"; Layout.fillWidth: true; font.bold: true }
                            Label { text: "Qty"; Layout.preferredWidth: 40; font.bold: true }
                            Label { text: "Price"; Layout.preferredWidth: 70; font.bold: true }
                            Label { text: "Total"; Layout.preferredWidth: 80; font.bold: true }
                        }
                    }

                    // The actual Items List
                    ListView {
                        id: itemsList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: m.selectedOrderItems // Binds to the Q_PROPERTY
                        delegate: ItemDelegate {
                            width: itemsList.width
                            height: 40
                            contentItem: RowLayout {
                                Label { text: modelData.product; Layout.fillWidth: true }
                                Label { text: modelData.qty; Layout.preferredWidth: 40 }
                                Label { text: modelData.price.toFixed(2); Layout.preferredWidth: 70 }
                                Label { text: modelData.total.toFixed(2); Layout.preferredWidth: 80; font.bold: true }
                            }
                        }
                    }
                }
            }

        }
    }
}
