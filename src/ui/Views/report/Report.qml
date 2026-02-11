import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

Item {
    id: root
    anchors.fill: parent

    // --- 1. THE MOCK ENGINE (RE-RANDOMIZES ALL POINTS) ---
    QtObject {
        id: mockEngine
        property double totalRevenue: 0
        property double totalOrders: 0
        property double totalCustomers: 0

        function randomize() {
            // KPI Values
            totalRevenue = 5000 + Math.random() * 15000;
            totalOrders = 500 + Math.random() * 2000;
            totalCustomers = 100 + Math.random() * 900;

            // Pie Chart (Donut)
            pieSeries.clear();
            var categories = ["Dine In", "To Go", "Delivery"];
            var colors = [window.theme.accent, window.theme.success, "#f39c12"];
            for (var i = 0; i < categories.length; i++) {
                var slice = pieSeries.append(categories[i], Math.random() * 500 + 100);
                slice.color = colors[i];
                slice.borderColor = "transparent";
            }

            // Line Chart (24 Hour Trend)
            revenueLine.clear();
            for (var hour = 0; hour <= 24; hour += 2) {
                // Generates a bell-curve style trend for lunch/dinner spikes
                var spike = (hour > 11 && hour < 14) || (hour > 18 && hour < 21) ? 2.5 : 1.0;
                revenueLine.append(hour, (Math.random() * 500 + 100) * spike);
            }

            // Report List
            reportModel.clear();
            var names = ["Eren J.", "Mikasa A.", "Armin A.", "Levi A.", "Sasha B.", "Erwin S."];
            var dishes = ["Spicy Beef", "Vegetable Ramen", "Miso Soup", "Gyoza", "Shrimp Tempura"];
            for (var j = 0; j < 6; j++) {
                reportModel.append({
                    "customer": names[Math.floor(Math.random() * names.length)],
                    "menu": dishes[Math.floor(Math.random() * dishes.length)],
                    "amount": 25 + Math.random() * 150,
                    "status": Math.random() > 0.3 ? "Completed" : "Preparing"
                });
            }
        }
    }

    Component.onCompleted: mockEngine.randomize()

    RowLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 25

        // --- LEFT COLUMN (65% Width) ---
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 20

            // Header Section
            RowLayout {
                Column {
                    Label { text: "Dashboard"; font.pixelSize: 28; font.bold: true; color: window.theme.textMain }
                    Label { text: "Real-time revenue and transaction tracking"; color: window.theme.textSecondary }
                }
                Item { Layout.fillWidth: true }

                Button {
                    text: "↻ Refresh Data"
                    onClicked: mockEngine.randomize()
                    background: Rectangle { color: parent.down ? window.theme.surfaceHighlight : window.theme.surface; radius: 8; border.color: window.theme.border }
                    contentItem: Text { text: parent.text; color: "white"; font.bold: true; padding: 10 }
                }
            }

            // KPI Row
            RowLayout {
                spacing: 15
                ReportCard { label: "Revenue"; value: "$" + mockEngine.totalRevenue.toLocaleString(Qt.locale(), 'f', 2); percent: "+12%"; isUp: true; icon: "" }
                ReportCard  { label: "Orders"; value: mockEngine.totalOrders.toFixed(0); percent: "-5%"; isUp: false; icon: "" }
                ReportCard  { label: "Customers"; value: mockEngine.totalCustomers.toFixed(0); percent: "+18%"; isUp: true; icon: "" }
            }

            // Line Chart: 24h Trend

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 300
                color: window.theme.sidePanelBg; radius: window.theme.cardRadius; border.color: window.theme.border

                ChartView {
                    anchors.fill: parent; backgroundColor: "transparent"; antialiasing: true
                    title: "Revenue Trend (Last 24 Hours)"; titleColor: "white"

                    ValueAxis { id: axisX; min: 0; max: 24; tickCount: 13; labelFormat: "%d:00"; labelsColor: window.theme.textSecondary }
                    ValueAxis { id: axisY; min: 0; max: 1500; labelsColor: window.theme.textSecondary; gridVisible: true; gridLineColor: window.theme.border }

                    LineSeries {
                        id: revenueLine
                        name: "Revenue ($)"
                        axisX: axisX; axisY: axisY
                        color: window.theme.accent; width: 3
                    }
                }
            }

            // Order Report Table
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: window.theme.sidePanelBg; radius: window.theme.cardRadius; border.color: window.theme.border
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20
                    Label { text: "Recent Transactions"; font.bold: true; color: "white"; font.pixelSize: 18 }
                    ListView {
                        id: reportList
                        Layout.fillHeight: true; Layout.fillWidth: true; clip: true
                        model: ListModel { id: reportModel }
                        delegate: ItemDelegate {
                            width: reportList.width; height: 45
                            contentItem: Row {
                                Label { text: customer; width: parent.width/4; color: "white" }
                                Label { text: menu; width: parent.width/4; color: window.theme.textSecondary }
                                Label { text: "$" + amount.toFixed(2); width: parent.width/4; color: "white" }
                                Label {
                                    text: status; color: status === "Completed" ? window.theme.success : window.theme.accent;
                                    font.bold: true; width: parent.width/4
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- RIGHT COLUMN (350px Fixed) ---
        ColumnLayout {
            Layout.preferredWidth: 350
            spacing: 20

            // Donut Chart

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 350
                color: window.theme.sidePanelBg; radius: window.theme.cardRadius; border.color: window.theme.border
                ChartView {
                    anchors.fill: parent; backgroundColor: "transparent"; antialiasing: true
                    legend.alignment: Qt.AlignBottom; legend.labelColor: "white"
                    PieSeries { id: pieSeries; holeSize: 0.6 }
                }
            }

            // Most Ordered List (Mocked)
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: window.theme.sidePanelBg; radius: window.theme.cardRadius; border.color: window.theme.border
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20
                    Label { text: "Top Performing Dishes"; font.bold: true; color: "white" }
                    Repeater {
                        model: 4
                        ItemDelegate {
                            Layout.fillWidth: true; height: 50
                            RowLayout {
                                anchors.fill: parent
                                Rectangle { width: 40; height: 40; radius: 8; color: window.theme.surfaceHighlight }
                                Column {
                                    Label { text: ["Spicy Miso", "Beef Bowl", "Gyoza Set", "Cooler Mix"][index]; color: "white"; font.bold: true }
                                    Label { text: (80 - index * 10) + " orders today"; color: window.theme.textSecondary; font.pixelSize: 11 }
                                }
                                Item { Layout.fillWidth: true }
                                Label { text: "+$" + (1200 - index * 200); color: window.theme.success; font.bold: true }
                            }
                        }
                    }
                }
            }
        }
    }
}
