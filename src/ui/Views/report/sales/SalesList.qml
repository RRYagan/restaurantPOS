import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: salesListViewRoot

    StackView {
        id: internalStack
        anchors.fill: parent
        initialItem: listViewComponent

        Component {
                    id: listViewComponent

                    ColumnLayout {
                        spacing: 10 // Reduced spacing for a tighter look

                        // --- FILTER SECTION (Unchanged) ---
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 110
                            Layout.margins: 15
                            color: theme.surface
                            radius: theme.cardRadius
                            border.color: theme.border

                            GridLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                columns: 4
                                columnSpacing: 15

                                TextField {
                                    id: searchField
                                    placeholderText: "🔍 Search ID, Ref or Staff..."
                                    Layout.fillWidth: true
                                    Layout.columnSpan: 2
                                    color: theme.textMain
                                    background: Rectangle { color: theme.surfaceHighlight; radius: 4 }
                                    onTextChanged: m.filterText = text
                                }

                                ComboBox {
                                    id: methodFilter
                                    model: ["All Methods", "Cash", "MOBILE MONEY", "Credit Card"]
                                    Layout.fillWidth: true
                                    onActivated: m.filterMethod = currentText
                                }

                                ComboBox {
                                    id: timeFilter
                                    model: ["Any Time", "Morning (6am-11am)", "Lunch (11am-3pm)", "Evening (3pm-10pm)"]
                                    Layout.fillWidth: true
                                    onActivated: m.filterTime = currentText
                                }

                                ComboBox {
                                    id: valueFilter
                                    model: ["Any Value", "High Value (>5k)", "Low Value (<1k)"]
                                    Layout.fillWidth: true
                                    onActivated: m.filterValue = currentText
                                }

                                Button {
                                    text: "Reset"
                                    Layout.fillWidth: true
                                    onClicked: {
                                        searchField.clear();
                                        methodFilter.currentIndex = 0;
                                        timeFilter.currentIndex = 0;
                                        valueFilter.currentIndex = 0;
                                        m.filterText = ""; m.filterMethod = "All Methods";
                                        m.filterTime = "Any Time"; m.filterValue = "Any Value";
                                    }
                                }
                            }
                        }

                        // --- TABLE SECTION ---
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.margins: 15
                            color: theme.sidePanelBg
                            radius: theme.cardRadius
                            border.color: theme.border
                            clip: true

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 0

                                // 1. HEADER (Updated to match Row Columns)
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 40
                                    color: theme.surfaceHighlight

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 10
                                        anchors.rightMargin: 10
                                        spacing: 10

                                        Label { text: "Date"; Layout.preferredWidth: 120; font.bold: true; color: theme.textSecondary }
                                        Label { text: "Order ID"; Layout.preferredWidth: 90; font.bold: true; color: theme.textSecondary }
                                        Label { text: "Staff"; Layout.preferredWidth: 90; font.bold: true; color: theme.textSecondary }
                                        Label { text: "Amount"; Layout.preferredWidth: 90; font.bold: true; color: theme.textSecondary }
                                        Label { text: "Status"; Layout.preferredWidth: 30; font.bold: true; color: theme.textSecondary } // Status
                                        Label { text: "Pay Ref"; Layout.preferredWidth: 100; font.bold: true; color: theme.textSecondary }
                                        Label { text: "Method"; Layout.fillWidth: true; font.bold: true; color: theme.textSecondary }
                                    }
                                }

                                // 2. LIST VIEW
                                ListView {
                                    id: salesListView
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    model: m.filteredData
                                    clip: true

                                    delegate: ItemDelegate {
                                        width: salesListView.width
                                        height: 45
                                        onClicked: {
                                            m.fetchOrderItems(modelData.order_id)
                                            internalStack.push(detailsComponent,
                                                               { "saleData": modelData })
                                        }

                                        background: Rectangle {
                                            color: hovered ? theme.surfaceHighlight : "transparent"
                                            opacity: 0.5
                                        }

                                        contentItem: RowLayout {
                                            spacing: 10

                                            // 1. Date (Fixed Width)
                                            Label {
                                                text: modelData.sale_date + " " + modelData.sale_hour + ":00"
                                                Layout.preferredWidth: 120
                                                color: theme.textSecondary
                                                font.pixelSize: 11
                                                elide: Text.ElideRight // Cuts off if too long
                                            }

                                            // 2. Order ID (Truncated + Tooltip)
                                            Label {
                                                text: modelData.order_id
                                                Layout.preferredWidth: 90
                                                color: modelData.status === "CLOSED" ? theme.success : theme.textMain
                                                font.bold: true
                                                font.pixelSize: 11
                                                elide: Text.ElideMiddle // Shows start and end of ID (e.g. "A12...9F")

                                                ToolTip.visible: maId.containsMouse
                                                ToolTip.text: modelData.order_id
                                                MouseArea { id: maId; anchors.fill: parent; hoverEnabled: true }
                                            }

                                            // 3. Staff (Truncated)
                                            Label {
                                                text: modelData.waiter_id
                                                Layout.preferredWidth: 90
                                                color: theme.textMain
                                                elide: Text.ElideRight
                                            }

                                            // 4. Amount (Formatted)
                                            Label {
                                                text: modelData.gross_amount.toLocaleString(Qt.locale(), 'f', 2)
                                                Layout.preferredWidth: 90
                                                color: theme.textMain
                                                font.bold: true
                                            }

                                            // 5. Status Dot
                                            Item {
                                                Layout.preferredWidth: 30
                                                Layout.fillHeight: true
                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    width: 10; height: 10; radius: 5
                                                    color: {
                                                        if (modelData.status === "OPEN") return "#FFC107" // Amber
                                                        return (modelData.signature !== "MISSING") ? theme.success : theme.danger
                                                    }
                                                }
                                            }

                                            // 6. Payment Ref (Truncated + Tooltip)
                                            Label {
                                                text: modelData.payment_id !== "" ? modelData.payment_id : "---"
                                                Layout.preferredWidth: 100
                                                color: theme.textSecondary
                                                font.pixelSize: 11
                                                elide: Text.ElideRight

                                                // Only show tooltip if there is actually text
                                                ToolTip.visible: maRef.containsMouse && text !== "---"
                                                ToolTip.text: text
                                                MouseArea { id: maRef; anchors.fill: parent; hoverEnabled: true }
                                            }

                                            // 7. Method (Fills remaining space)
                                            Label {
                                                text: modelData.status === "OPEN" ? "Pending" : modelData.method
                                                Layout.fillWidth: true
                                                color: theme.textSecondary
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }
                                }

                                // Footer (Unchanged)
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 40
                                    color: theme.surface
                                    border.color: theme.border
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 15
                                        Label { text: "Records: " + m.filteredData.length; font.bold: true; color: theme.textSecondary }
                                        Item { Layout.fillWidth: true }
                                        Label {
                                            text: "TOTAL: KES " + m.totalGross.toLocaleString(Qt.locale(), 'f', 2)
                                            font.bold: true; color: theme.success; Layout.rightMargin: 15
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
        // --- THE DETAILS PAGE COMPONENT ---
        Component {
            id: detailsComponent
            // Important: This filename must be SalesDetails.qml in the same folder
            SalesDetails {
                // Ensure SalesDetails.qml has a property 'saleData' and a signal 'back'
                saleData: saleData
                onBack: internalStack.pop()
            }
        }
    }
}
