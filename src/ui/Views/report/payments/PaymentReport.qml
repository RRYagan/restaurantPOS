import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: paymentHub
    property string pageTitle: "Payments Management"
    // anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- SUB-HEADER WITH DROPDOWN NAV ---
        Rectangle {
            Layout.fillWidth: true
            height: 70
            color: window.theme.surface
            border.color: window.theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 25; anchors.rightMargin: 25
                spacing: 20

                // The Navigation Dropdown
                RowLayout {
                    spacing: 12
                    Label {
                        text: "View:"
                        color: window.theme.textSecondary
                        font.bold: true
                    }

                    ComboBox {
                        id: viewSelector
                        Layout.preferredWidth: 220
                        model: ["Transaction List", "Payments Analytics", "Print & Share"]

                        currentIndex: subStack.currentIndex
                        onActivated: (index) => subStack.currentIndex = index

                        // Custom Styling for the Dropdown
                        contentItem: Text {
                            text: viewSelector.displayText
                            color: "white"
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 12
                        }

                        background: Rectangle {
                            color: window.theme.surfaceHighlight
                            radius: 8
                            border.color: window.theme.border
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Contextual Action Button (Changes based on view)
                Button {
                    text: subStack.currentIndex === 1 ? "📊 Refresh Charts" : "📥 Export Data"
                    visible: subStack.currentIndex !== 2 // Hide if on export page
                    onClicked: {
                        if(subStack.currentIndex === 0) { /* trigger list refresh */ }
                    }
                }
            }
        }

        // --- SUB-STACK CONTENT ---
        StackLayout {
            id: subStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0 // Controlled by the ComboBox above

            // Tab 0: List View
            PaymentList{ }

            // Tab 1: Analytics
            PaymentAnalytics{ }

            // Tab 2: Print/Share
            PaymentExport { }
        }
    }
}
