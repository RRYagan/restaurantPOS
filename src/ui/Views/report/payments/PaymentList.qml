import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: paymentsListView
    property string pageTitle: "Payment Transactions"
    // anchors.fill: parent

    // --- 1. DATA MODELS ---
    ListModel { id: masterModel }
    ListModel { id: filteredModel }

    // --- 2. FILTER LOGIC ---
    function applyFilters() {
        filteredModel.clear();

        var now = new Date();
        var filterDate = new Date();

        // Calculate the "start date" based on dropdown
        if (dateFilter.currentText === "Today") {
            filterDate.setHours(0, 0, 0, 0);
        } else if (dateFilter.currentText === "Last 7 Days") {
            filterDate.setDate(now.getDate() - 7);
        } else {
            filterDate = new Date(0); // All Time (Unix Epoch)
        }

        for (var i = 0; i < masterModel.count; i++) {
            var item = masterModel.get(i);
            var itemDate = new Date(item.created_at);

            // Filter Conditions
            var matchMethod = methodFilter.currentText === "All Methods" || item.payment_type === methodFilter.currentText;
            var matchStatus = statusFilter.currentText === "All Statuses" || item.status === statusFilter.currentText;
            var matchTable = tableFilter.currentText === "All Tables" || item.user_tag === tableFilter.currentText;
            var matchDate = itemDate >= filterDate;

            if (matchMethod && matchStatus && matchTable && matchDate) {
                filteredModel.append(item);
            }
        }
    }

    Component.onCompleted: {
        const types = ["Credit Card", "Cash", "Digital Wallet"];
        const statuses = ["Completed", "Failed", "Initiated"];
        const tables = ["Table 1", "Table 2", "Table 3"];

        // Generate mock data across different dates
        for (let i = 0; i < 40; i++) {
            let d = new Date();
            d.setDate(d.getDate() - Math.floor(Math.random() * 10)); // Random dates within last 10 days

            masterModel.append({
                "id": "PAY-" + (1000 + i),
                "order_id": "ORD-" + (7700 + i),
                "payment_type": types[Math.floor(Math.random() * types.length)],
                "amount_cents": Math.floor(Math.random() * 15000) + 1000,
                "status": statuses[Math.floor(Math.random() * statuses.length)],
                "user_tag": tables[Math.floor(Math.random() * tables.length)],
                "created_at": d.toISOString().replace('T', ' ').substring(0, 19)
            });
        }
        applyFilters();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        // --- Filter Bar with Reset Button ---
        Rectangle {
            Layout.fillWidth: true
            height: 100
            color: window.theme.sidePanelBg
            radius: window.theme.cardRadius
            border.color: window.theme.border

            GridLayout {
                anchors.fill: parent
                anchors.margins: 15
                columns: 5 // Increased columns to fit the button
                rowSpacing: 10; columnSpacing: 20

                // Row 1: Labels
                Label { text: "Date Range"; color: window.theme.textSecondary }
                Label { text: "Method"; color: window.theme.textSecondary }
                Label { text: "Status"; color: window.theme.textSecondary }
                Label { text: "Location/Table"; color: window.theme.textSecondary }
                Item { } // Empty cell above the button

                // Row 2: Controls
                ComboBox {
                    id: dateFilter
                    Layout.fillWidth: true
                    model: ["All Time", "Today", "Last 7 Days"]
                    onActivated: applyFilters()
                }

                ComboBox {
                    id: methodFilter
                    Layout.fillWidth: true
                    model: ["All Methods", "Credit Card", "Cash", "Digital Wallet"]
                    onActivated: applyFilters()
                }

                ComboBox {
                    id: statusFilter
                    Layout.fillWidth: true
                    model: ["All Statuses", "Completed", "Failed", "Initiated"]
                    onActivated: applyFilters()
                }

                ComboBox {
                    id: tableFilter
                    Layout.fillWidth: true
                    model: ["All Tables", "Table 1", "Table 2", "Table 3"]
                    onActivated: applyFilters()
                }

                // --- THE RESET BUTTON ---
                Button {
                    text: "↺ Reset Filters"
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40

                    contentItem: Text {
                        text: parent.text
                        color: parent.down ? window.theme.textSecondary : "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.hovered ? window.theme.surfaceHighlight : window.theme.surface
                        border.color: window.theme.border
                        radius: 8
                    }

                    onClicked: {
                        // Reset all indices to 0 ("All" options)
                        dateFilter.currentIndex = 0;
                        methodFilter.currentIndex = 0;
                        statusFilter.currentIndex = 0;
                        tableFilter.currentIndex = 0;

                        // Re-run the filter logic to show all data
                        applyFilters();
                    }
                }
            }
        }
        // --- 4. DATA TABLE ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: window.theme.sidePanelBg
            radius: window.theme.cardRadius
            border.color: window.theme.border
            clip: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Table Header
                Rectangle {
                    Layout.fillWidth: true; height: 45
                    color: window.theme.surfaceHighlight
                    Row {
                        anchors.fill: parent; anchors.leftMargin: 20; spacing: 10
                        Label { text: "Date"; width: parent.width * 0.2; color: window.theme.textSecondary; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                        Label { text: "Order ID"; width: parent.width * 0.15; color: window.theme.textSecondary; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                        Label { text: "Method"; width: parent.width * 0.15; color: window.theme.textSecondary; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                        Label { text: "Amount"; width: parent.width * 0.15; color: window.theme.textSecondary; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                        Label { text: "Status"; width: parent.width * 0.15; color: window.theme.textSecondary; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                        Label { text: "Table"; width: parent.width * 0.1; color: window.theme.textSecondary; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                    }
                }

                ListView {
                    id: paymentsList
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: filteredModel
                    delegate: ItemDelegate {
                        width: paymentsList.width; height: 50
                        contentItem: Row {
                            anchors.fill: parent; anchors.leftMargin: 20; spacing: 10
                            Label { text: model.created_at; width: parent.width * 0.2; color: "white" }
                            Label { text: model.order_id; width: parent.width * 0.15; color: window.theme.textSecondary }
                            Label { text: model.payment_type; width: parent.width * 0.15; color: "white" }
                            Label { text: "$" + (model.amount_cents / 100).toFixed(2); width: parent.width * 0.15; color: "white"; font.bold: true }
                            Label {
                                text: model.status
                                width: parent.width * 0.15
                                color: model.status === "Completed" ? window.theme.success : (model.status === "Failed" ? "#ff4444" : "#f39c12")
                                font.bold: true
                            }
                            Label { text: model.user_tag; width: parent.width * 0.1; color: "white" }
                        }
                    }
                }
            }
        }
    }
}
