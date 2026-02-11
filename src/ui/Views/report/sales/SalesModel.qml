import QtQuick

QtObject {
    id: root
        property var sourceModel: null

        // --- Filter States (Bind these to UI controls) ---
        property string filterText: ""
        property string filterMethod: "All Methods"
        property string filterTime: "Any Time"
        property string filterValue: "Any Value"

        // --- KPI Properties ---
        property double todayRevenue: 0
        property double avgDailyRevenue: 0
        property double revenueGrowth: 0
        property int missingKraCount: 0

        // --- List Properties ---
        property var filteredData: []
        property double totalNet: 0.0
        property double totalGross: 0.0

        // --- Visualization Models ---
        property var dailyTrendModel: []
        property var ticketSizeModel: []
        property var hourlyHeatmapModel: []
        property var staffPerformance: []
        property var paymentMixModel: []

    readonly property var rawData: [
            {"id": "S0001", "user_name": "Sarah", "sale_date": "2026-02-01 12:30:00", "total_amount": 1250.00, "kra_sig": "SIG1", "payment_method": "M-Pesa"},
            {"id": "S0002", "user_name": "Mike", "sale_date": "2026-02-01 19:45:00", "total_amount": 4500.00, "kra_sig": "SIG2", "payment_method": "Cash"},
            {"id": "S0003", "user_name": "John", "sale_date": "2026-02-01 13:15:00", "total_amount": 850.50, "kra_sig": "SIG3", "payment_method": "M-Pesa"},
            {"id": "S0004", "user_name": "Lisa", "sale_date": "2026-02-01 20:10:00", "total_amount": 3200.00, "kra_sig": "SIG4", "payment_method": "Credit Card"},
            {"id": "S0005", "user_name": "Sarah", "sale_date": "2026-02-02 12:45:00", "total_amount": 1100.00, "kra_sig": "SIG5", "payment_method": "Cash"},
            {"id": "S0006", "user_name": "Mike", "sale_date": "2026-02-02 18:30:00", "total_amount": 550.00, "kra_sig": "SIG6", "payment_method": "M-Pesa"},
            {"id": "S0007", "user_name": "John", "sale_date": "2026-02-02 14:00:00", "total_amount": 4200.00, "kra_sig": null, "payment_method": "Cash"}, // Missing Sig for Audit
            {"id": "S0008", "user_name": "Lisa", "sale_date": "2026-02-02 21:15:00", "total_amount": 950.00, "kra_sig": "SIG8", "payment_method": "M-Pesa"},
            {"id": "S0009", "user_name": "Sarah", "sale_date": "2026-02-03 12:15:00", "total_amount": 2500.00, "kra_sig": "SIG9", "payment_method": "Credit Card"},
            {"id": "S0010", "user_name": "Mike", "sale_date": "2026-02-03 19:00:00", "total_amount": 750.00, "kra_sig": "SIG10", "payment_method": "Cash"},
            {"id": "S0011", "user_name": "John", "sale_date": "2026-02-03 13:45:00", "total_amount": 1800.00, "kra_sig": "SIG11", "payment_method": "M-Pesa"},
            {"id": "S0012", "user_name": "Lisa", "sale_date": "2026-02-04 18:20:00", "total_amount": 450.00, "kra_sig": "SIG12", "payment_method": "Cash"},
            {"id": "S0013", "user_name": "Sarah", "sale_date": "2026-02-04 12:55:00", "total_amount": 6200.00, "kra_sig": "SIG13", "payment_method": "M-Pesa"},
            {"id": "S0014", "user_name": "Mike", "sale_date": "2026-02-05 20:30:00", "total_amount": 300.00, "kra_sig": null, "payment_method": "Cash"}, // Missing Sig
            {"id": "S0015", "user_name": "John", "sale_date": "2026-02-05 13:10:00", "total_amount": 1500.00, "kra_sig": "SIG15", "payment_method": "Credit Card"},
            {"id": "S0016", "user_name": "Lisa", "sale_date": "2026-02-06 19:45:00", "total_amount": 8500.00, "kra_sig": "SIG16", "payment_method": "M-Pesa"}, // Weekend Peak
            {"id": "S0017", "user_name": "Sarah", "sale_date": "2026-02-06 12:05:00", "total_amount": 1200.00, "kra_sig": "SIG17", "payment_method": "Cash"},
            {"id": "S0018", "user_name": "Mike", "sale_date": "2026-02-07 18:55:00", "total_amount": 3200.00, "kra_sig": "SIG18", "payment_method": "M-Pesa"},
            {"id": "S0019", "user_name": "John", "sale_date": "2026-02-07 14:15:00", "total_amount": 5400.00, "kra_sig": "SIG19", "payment_method": "Credit Card"},
            {"id": "S0020", "user_name": "Lisa", "sale_date": "2026-02-08 20:00:00", "total_amount": 1100.00, "kra_sig": "SIG20", "payment_method": "Cash"},
            {"id": "S0021", "user_name": "Sarah", "sale_date": "2026-02-09 12:30:00", "total_amount": 950.00, "kra_sig": "SIG21", "payment_method": "M-Pesa"},
            {"id": "S0022", "user_name": "Mike", "sale_date": "2026-02-10 19:15:00", "total_amount": 4200.00, "kra_sig": "SIG22", "payment_method": "Cash"}
            // ... more data generated internally following this pattern ...
        ]
    function exportToCSV() {
            let csvString = "Transaction ID,User,Date,Amount,KRA Signature,Payment Method\n";
            // Export ONLY what is currently visible in the filtered list
            let data = filteredData;

            for (let i = 0; i < data.length; i++) {
                let row = data[i];
                csvString += `${row.orderId},${row.staff},${row.displayDate},${row.gross},${row.signature || "MISSING"},${row.method}\n`;
            }

            console.log("--- CSV EXPORT (FILTERED) ---");
            console.log(csvString);
            return csvString;
        }

        function refresh() {
            let data = (sourceModel && sourceModel.count > 0) ? sourceModel : rawData;
            let isListModel = (sourceModel && sourceModel.count > 0);
            let count = isListModel ? data.count : data.length;

            // Reset Local Accumulators
            let dailyMap = {};
            let hourMap = Array(24).fill(0);
            let staffMap = {};
            let sizeMap = {"0-500":0, "501-1500":0, "1501-3000":0, "3001+":0};
            let kraErrors = 0;

            let displayList = [];
            let tempNet = 0;
            let tempGross = 0;

            for (let i = 0; i < count; i++) {
                let row = isListModel ? data.get(i) : data[i];

                // --- 1. Filter Logic ---
                let sTxt = filterText.toLowerCase();
                let matchSearch = sTxt === "" || row.id.toLowerCase().includes(sTxt) || row.user_name.toLowerCase().includes(sTxt);

                let matchMethod = filterMethod === "All Methods" || row.payment_method === filterMethod;

                let dtParts = row.sale_date.split(" ");
                let hour = parseInt(dtParts[1].split(":")[0]);
                let matchTime = true;
                if (filterTime === "Morning (6am-11am)") matchTime = (hour >= 6 && hour < 11);
                else if (filterTime === "Lunch (11am-3pm)") matchTime = (hour >= 11 && hour < 15);
                else if (filterTime === "Evening (3pm-10pm)") matchTime = (hour >= 15 && hour < 22);

                let amt = row.total_amount;
                let matchValue = true;
                if (filterValue === "High Value (>5k)") matchValue = (amt > 5000);
                else if (filterValue === "Low Value (<1k)") matchValue = (amt < 1000);

                // --- 2. Process Matches ---
                if (matchSearch && matchMethod && matchTime && matchValue) {
                    // List Data
                    let netRow = amt / 1.16;
                    tempNet += netRow;
                    tempGross += amt;

                    displayList.push({
                        "displayDate": row.sale_date,
                        "orderId": row.id,
                        "staff": row.user_name,
                        "gross": amt,
                        "net": netRow,
                        "signature": row.kra_sig || "",
                        "method": row.payment_method
                    });

                    // Analytics Data (Only aggregated if matched by filters)
                    let date = dtParts[0];
                    dailyMap[date] = (dailyMap[date] || 0) + amt;
                    hourMap[hour] += amt;
                    if (!staffMap[row.user_name]) staffMap[row.user_name] = {val:0, hits:0};
                    staffMap[row.user_name].val += amt;
                    staffMap[row.user_name].hits++;

                    if (amt <= 500) sizeMap["0-500"]++;
                    else if (amt <= 1500) sizeMap["501-1500"]++;
                    else if (amt <= 3000) sizeMap["1501-3000"]++;
                    else sizeMap["3001+"]++;

                    if (!row.kra_sig) kraErrors++;
                }
            }

            // --- 3. Finalize Properties ---
            filteredData = displayList;
            totalNet = tempNet;
            totalGross = tempGross;
            missingKraCount = kraErrors;

            // Update Trend
            let trend = [];
            let dates = Object.keys(dailyMap).sort();
            dates.forEach(d => trend.push({"label": d.substring(5), "value": dailyMap[d]}));
            dailyTrendModel = trend;

            // Update KPIs
            todayRevenue = dailyMap[dates[dates.length-1]] || 0;
            let totalRev = trend.reduce((a,b) => a + b.value, 0);
            avgDailyRevenue = trend.length > 0 ? totalRev / trend.length : 0;
            revenueGrowth = avgDailyRevenue > 0 ? ((todayRevenue - avgDailyRevenue) / avgDailyRevenue) * 100 : 0;

            // Update Heatmap
            let heatmap = [];
            let maxRevInHour = Math.max(...hourMap.slice(8, 22), 1);
            for(let h = 8; h <= 21; h++) {
                let displayHour = (h > 12 ? h-12 : h) + (h >= 12 ? " PM" : " AM");
                heatmap.push({ "time": displayHour, "ratio": hourMap[h] / maxRevInHour, "total": hourMap[h] });
            }
            hourlyHeatmapModel = heatmap;

            // Update Staff Performance
            let staff = [];
            for (let name in staffMap) {
                staff.push({ "name": name, "total": staffMap[name].val, "avg": staffMap[name].val / staffMap[name].hits });
            }
            staffPerformance = staff.sort((a,b) => b.total - a.total);

            // Update Histogram
            let hist = [];
            for (let range in sizeMap) hist.push({"label": range, "count": sizeMap[range]});
            ticketSizeModel = hist;
        }

        // --- 4. Auto-Refresh Triggers ---
        onFilterTextChanged: refresh()
        onFilterMethodChanged: refresh()
        onFilterTimeChanged: refresh()
        onFilterValueChanged: refresh()

        Component.onCompleted: refresh()
    }
