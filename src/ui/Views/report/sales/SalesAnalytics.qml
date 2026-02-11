import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: viewRoot
    contentWidth: availableWidth
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    // Smooth background for the scroll area
    background: Rectangle { color: "transparent" }

    ColumnLayout {
        width: parent.width - 40
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 25
        anchors.topMargin: 20

        // GROUP 1: HEALTH CARDS
        RowLayout {
            spacing: 20
            AnalyticsCard {
                title: "Today's Revenue"; value: "KES " + m.todayRevenue.toLocaleString()
                color: theme.textMain; icon: "💰"
            }
            AnalyticsCard {
                title: "Daily Avg"; value: "KES " + m.avgDailyRevenue.toLocaleString()
                color: theme.textMain; icon: "📈"
            }
            AnalyticsCard {
                title: "Revenue Growth"; value: m.revenueGrowth.toFixed(1) + "%"
                // Dynamically color text based on performance
                color: m.revenueGrowth >= 0 ? theme.success : theme.danger; icon: "📊"
            }
        }

        // MIDDLE ROW: Trend & Staff
        RowLayout {
            Layout.preferredHeight: 300; spacing: 20

            // Daily Revenue Trend Line
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                radius: theme.cardRadius; color: theme.surface; border.color: theme.border

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16
                    Label { text: "10-Day Revenue Trend"; font.bold: true; color: theme.textMain }
                    Row {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10; Layout.topMargin: 10
                        Repeater {
                            model: m.dailyTrendModel
                            delegate: Column {
                                anchors.bottom: parent.bottom; spacing: 5
                                Rectangle {
                                    width: 30; height: (modelData.value / (m.avgDailyRevenue*2)) * 150
                                    color: theme.accent; radius: 4 // Using Accent color for bars
                                }
                                Label { text: modelData.label; font.pixelSize: 9; color: theme.textSecondary; anchors.horizontalCenter: parent.horizontalCenter }
                            }
                        }
                    }
                }
            }

            // Staff Performance Leaderboard
            Rectangle {
                width: 350; Layout.fillHeight: true
                radius: theme.cardRadius; color: theme.surface; border.color: theme.border

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16
                    Label { text: "Staff Efficiency (Avg Ticket)"; font.bold: true; color: theme.textMain }
                    ListView {
                        Layout.fillWidth: true; Layout.fillHeight: true; model: m.staffPerformance; clip: true
                        delegate: ItemDelegate {
                            width: parent.width
                            background: Rectangle { color: hovered ? theme.surfaceHighlight : "transparent"; radius: 4 }
                            contentItem: RowLayout {
                                Label { text: modelData.name; color: theme.textMain; Layout.fillWidth: true }
                                Label { text: "Avg: " + Math.round(modelData.avg); color: theme.success; font.bold: true }
                            }
                        }
                    }
                }
            }
        }

        // BOTTOM ROW: Heatmap & Audit
        RowLayout {
            Layout.preferredHeight: 250; spacing: 20

            // Hourly Sales Heatmap
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                radius: theme.cardRadius; color: theme.surface; border.color: theme.border

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16; spacing: 12
                    Label { text: "Hourly Peak Sales Heatmap"; font.bold: true; color: theme.textMain; font.pixelSize: 14 }

                    RowLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 6
                        Repeater {
                            model: m.hourlyHeatmapModel
                            delegate: ColumnLayout {
                                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 8
                                Rectangle {
                                    id: heatBlock
                                    Layout.fillWidth: true; Layout.fillHeight: true; radius: 4
                                    // Use theme.success (green) with alpha based on sales density
                                    color: modelData.ratio > 0
                                           ? Qt.rgba(theme.success.r, theme.success.g, theme.success.b, Math.max(modelData.ratio, 0.15))
                                           : theme.surfaceHighlight

                                    border.width: modelData.ratio > 0.8 ? 1 : 0
                                    border.color: theme.textMain

                                    MouseArea { id: ma; anchors.fill: parent; hoverEnabled: true }

                                    ToolTip {
                                        visible: ma.containsMouse; delay: 50
                                        text: "<b>" + modelData.time + "</b><br>Total: KES " + modelData.total.toLocaleString()
                                        contentItem: Text { text: parent.text; color: "#fff"; font.pixelSize: 11 }
                                        background: Rectangle { color: "#333"; radius: 4 }
                                    }
                                }
                                Label {
                                    text: modelData.time.split(" ")[0]
                                    font.pixelSize: 9
                                    color: modelData.ratio > 0.7 ? theme.success : theme.textSecondary
                                    Layout.alignment: Qt.AlignHCenter
                                    visible: index % 2 === 0
                                }
                            }
                        }
                    }
                }
            }

            // Audit Risk Card
            Rectangle {
                width: 350; Layout.fillHeight: true; radius: theme.cardRadius; color: theme.surface; border.color: theme.border
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16; spacing: 15
                    Label { text: "Compliance Audit"; font.bold: true; color: theme.textMain }

                    Rectangle {
                        Layout.fillWidth: true; height: 60; radius: 8
                        // Mix theme.danger/success with background for a subtle alert look
                        color: m.missingKraCount > 0 ? Qt.rgba(theme.danger.r, theme.danger.g, theme.danger.b, 0.2)
                                                     : Qt.rgba(theme.success.r, theme.success.g, theme.success.b, 0.2)
                        RowLayout {
                            anchors.centerIn: parent; spacing: 10
                            Text { text: m.missingKraCount > 0 ? "⚠️" : "✅"; font.pixelSize: 24 }
                            Column {
                                Label { text: "eTIMS Coverage"; font.bold: true; color: theme.textMain }
                                Label { text: m.missingKraCount + " Missing Signatures"; color: theme.textSecondary; font.pixelSize: 11 }
                            }
                        }
                    }

                    Label { text: "Ticket Size Distribution"; color: theme.textSecondary; font.pixelSize: 12 }
                    RowLayout {
                        spacing: 15; Layout.fillWidth: true
                        Repeater {
                            model: m.ticketSizeModel
                            delegate: Column {
                                spacing: 4
                                Rectangle { width: 40; height: Math.max(modelData.count * 5, 2); color: theme.accent; radius: 2 }
                                Label { text: modelData.label; font.pixelSize: 8; color: theme.textSecondary }
                            }
                        }
                    }
                }
            }
        }
    }
}
