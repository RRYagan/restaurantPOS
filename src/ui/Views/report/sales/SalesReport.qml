import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Item {
    id: salesHub
    property string pageTitle: "Sales & Revenue Report"
    ReportController {
            id: m
        }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Navigation Header
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "transparent"
            border.width: 0
            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12

                // 1. Navigation View Selector
                ComboBox {
                    id: viewSelector
                    model: ["Sales List", "Analytics"]
                    onActivated: (index) => salesStack.currentIndex = index

                    background: Rectangle {
                        implicitWidth: 140
                        color: theme.surfaceHighlight // Themed
                        radius: 6
                    }
                    contentItem: Text {
                        text: viewSelector.displayText
                        color: theme.textMain // Themed
                        padding: 10
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Item { Layout.fillWidth: true }

                // 2. Export Actions
                ComboBox {
                    id: exportDropdown
                    model: ["CSV", "PDF", "Print"]
                    currentIndex: -1
                    displayText: "Export"

                    background: Rectangle {
                        implicitWidth: 100
                        color: theme.surfaceHighlight // Themed
                        radius: 6
                        border.color: theme.border // Themed
                    }

                    contentItem: Text {
                        text: exportDropdown.displayText
                        color: theme.success // Themed green
                        padding: 10
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    onActivated: (index) => {
                        // Accessing the 'm' id defined above
                        if (index === 0) m.exportToCSV();
                        else if (index === 1) m.generatePDFReport();
                        else m.printView();

                        currentIndex = -1;
                    }
                }
            }
        }

        // --- View Content ---
        StackLayout {
            id: salesStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0

            // These views will automatically use 'm' and 'theme' from parent scope
            SalesList { }

            SalesAnalytics {}
        }
    }
}
