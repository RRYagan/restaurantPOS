import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: dashboardShell

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // --- NAVIGATION HEADER (Stays Static) ---
        RowLayout {
            Layout.fillWidth: true

            Column {
                Label {
                    text: mainStack.currentItem.pageTitle || "Analytics"
                    font.pixelSize: 28; font.bold: true; color: window.theme.textMain
                }
                Label {
                    text: "Real-time data management"
                    color: window.theme.textSecondary
                }
            }

            Item { Layout.fillWidth: true }

            ComboBox {
                id: navigationCombo
                model: ListModel {
                    ListElement { name: "Main Report"; page: "Report.qml" }
                    ListElement { name: "Sales Report"; page: "sales/SalesReport.qml" }
                    ListElement { name: "PaymentsReport"; page: "payments/PaymentReport.qml" }
                    ListElement { name: "Expenditure Report"; page: "ExpenditureReport.qml" }
                    ListElement { name: "Inventory"; page: "InventoryReport.qml" }
                }
                textRole: "name"
                onActivated: (index) => mainStack.replace(model.get(index).page)

                background: Rectangle {
                    implicitWidth: 200; implicitHeight: 40
                    color: window.theme.surface; border.color: window.theme.border; radius: 8
                }
                contentItem: Text {
                    text: navigationCombo.displayText; color: "white"
                    verticalAlignment: Text.AlignVCenter; leftPadding: 10
                }
            }
        }

        // --- DYNAMIC CONTENT AREA ---
        StackView {
            id: mainStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: "Report.qml" // This is the code you provided in your first prompt

            // Transitions
            replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
            replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
        }
    }
}
