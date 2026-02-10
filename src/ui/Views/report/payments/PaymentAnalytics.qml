import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

ScrollView {
    id: analyticsRoot
    anchors.fill: parent
    contentWidth: availableWidth
    clip: true

    // Function moved inside for scope safety
    function updateAnalyticsScale(scale) {
        if (scale === "Day") {
            axisX.categories = ["8am", "11am", "2pm", "5pm", "8pm", "11pm"];
            axisY.max = 1500;
        } else if (scale === "Week") {
            axisX.categories = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
            axisY.max = 6000;
        } else if (scale === "Month") {
            axisX.categories = ["Week 1", "Week 2", "Week 3", "Week 4"];
            axisY.max = 25000;
        }
    }

    ColumnLayout {
        width: analyticsRoot.availableWidth
        spacing: 25
        anchors.margins: 25

        // --- 1. TOP ANALYTICS CONTROL BAR ---
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Volume Analysis"
                font.pixelSize: 22; font.bold: true; color: "white"
            }
            Item { Layout.fillWidth: true }

            ButtonGroup { id: timeGroup }
            Row {
                spacing: 2
                Repeater {
                    model: ["Day", "Week", "Month"]
                    Button {
                        text: modelData
                        checkable: true
                        checked: index === 0
                        ButtonGroup.group: timeGroup
                        onClicked: updateAnalyticsScale(modelData)

                        background: Rectangle {
                            implicitWidth: 80; implicitHeight: 32
                            color: parent.checked ? window.theme.accent : window.theme.surface
                            border.color: window.theme.border
                            radius: 4
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"; font.bold: true
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }

        // --- 2. MAIN VOLUME GRAPH ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 500
            color: window.theme.sidePanelBg
            radius: window.theme.cardRadius
            border.color: window.theme.border

            ChartView {
                id: mainChart
                anchors.fill: parent
                anchors.margins: 5 // Reduced margins to maximize bar space
                backgroundColor: "transparent"
                antialiasing: true
                legend.alignment: Qt.AlignBottom
                legend.labelColor: "white"

                // Remove padding around the plotting area
                margins.top: 20
                margins.bottom: 10
                margins.left: 10
                margins.right: 10

                BarCategoryAxis {
                    id: axisX
                    categories: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
                    labelsColor: window.theme.textSecondary
                    gridVisible: false // Cleaner look makes bars pop more
                }

                ValueAxis {
                    id: axisY
                    min: 0
                    max: 6000
                    labelFormat: "$%.0f"
                    labelsColor: window.theme.textSecondary
                    tickCount: 6 // Keeps the horizontal lines clean
                }

                // Previous Period (Comparison - Thin Bars)
                BarSeries {
                    id: prevSeries
                    name: "Previous Period"
                    axisX: axisX
                    axisY: axisY
                    barWidth: 0.7 // Scales relative to the category width (0.0 to 1.0)

                    BarSet {
                        values: [3200, 3100, 3400, 3000, 4200, 4800, 4500]
                        color: window.theme.surfaceHighlight
                    }
                }

                // Current Period (Main Focus - Shares the same category space)
                BarSeries {
                    id: currSeries
                    name: "Current Period"
                    axisX: axisX
                    axisY: axisY
                    barWidth: 0.7 // Adjusting this makes bars wider/thinner

                    BarSet {
                        values: [3500, 3800, 3300, 3900, 4900, 5200, 5100]
                        color: window.theme.accent
                    }
                }

                LineSeries {
                    name: "Target Trend"
                    color: window.theme.success
                    width: 3
                    XYPoint { x: 0; y: 3000 }
                    XYPoint { x: 6; y: 5800 }
                }
            }
        }
        // --- 3. BUSINESS INSIGHT CARDS ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 120
                color: window.theme.sidePanelBg; radius: 12; border.color: window.theme.border
                ColumnLayout {
                    anchors.centerIn: parent
                    Label { text: "AVG. TICKET SIZE"; color: window.theme.textSecondary; font.pixelSize: 12 }
                    Label { text: "$42.50"; color: "white"; font.pixelSize: 28; font.bold: true }
                    Label { text: "↑ 4.2% vs last month"; color: window.theme.success; font.pixelSize: 12 }
                }
            }

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 120
                color: window.theme.sidePanelBg; radius: 12; border.color: window.theme.border
                ColumnLayout {
                    anchors.centerIn: parent
                    Label { text: "PEAK REVENUE HOUR"; color: window.theme.textSecondary; font.pixelSize: 12 }
                    Label { text: "19:00 - 20:00"; color: "white"; font.pixelSize: 24; font.bold: true }
                    Label { text: "$1,240.00 avg volume"; color: window.theme.textSecondary; font.pixelSize: 12 }
                }
            }
        }

        // --- 4. TABLE & METHOD HEATMAPS (Formerly Section 2) ---
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 400
            spacing: 20

            // Zone Utilization
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: window.theme.sidePanelBg; radius: window.theme.cardRadius; border.color: window.theme.border

                ChartView {
                    anchors.fill: parent; backgroundColor: "transparent"
                    title: "Revenue by Zone"; titleColor: "white"; antialiasing: true
                    PieSeries {
                        PieSlice { label: "Main Hall"; value: 55; color: "#3498db"; exploded: true }
                        PieSlice { label: "Bar"; value: 25; color: window.theme.accent }
                        PieSlice { label: "Patio"; value: 20; color: window.theme.success }
                    }
                }
            }

            // Health Metrics
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: window.theme.sidePanelBg; radius: window.theme.cardRadius; border.color: window.theme.border

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 25
                    Label { text: "Operational Health"; font.bold: true; color: "white"; font.pixelSize: 18 }

                    StatusMetric { label: "Authorization Rate"; value: 0.98; color: window.theme.success }
                    StatusMetric { label: "Avg. Transaction Time"; value: 0.45; color: "#3498db" }
                    StatusMetric { label: "Refund Ratio"; value: 0.02; color: "#e74c3c" }

                    Item { Layout.fillHeight: true }

                    Rectangle {
                        Layout.fillWidth: true; height: 60; color: window.theme.surface; radius: 8
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 10
                            Column {
                                Label { text: "Busiest Window"; color: window.theme.textSecondary; font.pixelSize: 10 }
                                Label { text: "6:00 PM - 7:30 PM"; color: "white"; font.bold: true }
                            }
                            Item { Layout.fillWidth: true }
                            Label { text: "🔥 High Load"; color: window.theme.accent; font.bold: true }
                        }
                    }
                }
            }
        }
    }
}
