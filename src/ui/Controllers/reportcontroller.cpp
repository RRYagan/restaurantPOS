#include "reportcontroller.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
ReportController::ReportController(QObject *parent)
    : QObject(parent)
{
    // Initialize default date range (e.g., last 30 days)
    m_startDate = QDate::currentDate().addDays(-30);
    m_endDate = QDate::currentDate();

    // Initial data load
    processData();
}

void ReportController::processData()
{m_totalGross = 0;
    m_missingKraCount = 0;
    m_filteredData.clear();

    // --- DEBUG: STAGE 1 (Variable Integrity) ---
    qDebug() << "--- START PROCESS DATA ---";
    qDebug() << "Start Date Obj:" << m_startDate << " (Valid:" << m_startDate.isValid() << ")";
    qDebug() << "End Date Obj:  " << m_endDate   << " (Valid:" << m_endDate.isValid() << ")";

    QSqlQuery query;
    const QString sql = "SELECT order_id, waiter_id, order_status, gross_amount, "
                        "sale_date, sale_hour, kra_receipt_number, payment_id, payment_method "
                        "FROM vw_sales_report "
                        "WHERE sale_date >= :start AND sale_date <= :end";

    if (!query.prepare(sql)) {
        qCritical() << "PREPARE FAIL:" << query.lastError().text();
        return;
    }

    // Bind values
    QString startStr = m_startDate.toString("yyyy-MM-dd");
    QString endStr   = m_endDate.toString("yyyy-MM-dd");
    query.bindValue(":start", startStr);
    query.bindValue(":end", endStr);

    // --- NEW BULLETPROOF DEBUG ---
    QVariantList list = query.boundValues();
    qDebug() << "--- Query Diagnostics ---";
    qDebug() << "Total Bound Parameters detected by Qt:" << list.count();

    for (int i = 0; i < list.count(); ++i) {
        qDebug() << "  Param" << i << ":" << list.at(i).toString()
        << "(Type:" << list.at(i).typeName() << ")";
    }

    if (!query.exec()) {
        qCritical() << "EXECUTION FAIL!";
        qCritical() << "Driver Error:" << query.lastError().driverText();
        qCritical() << "DB Error:    " << query.lastError().databaseText();
        qDebug() << "Attempted SQL:" << query.executedQuery();
        return;
    }

    int rowsFound = 0;
    while (query.next()) {
        rowsFound++;
        // --- 2. EXTRACT DATA ---
        QString orderId = query.value("order_id").toString();
        QString waiter  = query.value("waiter_id").toString();
        QString status  = query.value("order_status").toString();
        QString payId   = query.value("payment_id").toString();
        QString method  = query.value("payment_method").toString();
        double gross    = query.value("gross_amount").toDouble();

        // Time parsing for filtering
        QString hourStr = query.value("sale_hour").toString();
        int hour = hourStr.toInt();

        // --- 3. APPLY FILTERS (IN C++) ---

        // A. Search Text (Checks Order ID, Waiter, or Payment ID)
        bool matchText = m_filterText.isEmpty() ||
                         orderId.contains(m_filterText, Qt::CaseInsensitive) ||
                         waiter.contains(m_filterText, Qt::CaseInsensitive) ||
                         payId.contains(m_filterText, Qt::CaseInsensitive);

        // B. Method Filter (e.g., "Cash", "M-Pesa")
        bool matchMethod = (m_filterMethod == "All Methods") ||
                           (method.contains(m_filterMethod, Qt::CaseInsensitive));

        // C. Value Filter
        bool matchValue = true;
        if (m_filterValue == "High Value (>5k)") matchValue = (gross > 5000);
        else if (m_filterValue == "Low Value (<1k)") matchValue = (gross < 1000);

        // D. Time Filter
        bool matchTime = true;
        if (m_filterTime != "Any Time") {

            // Robust parsing: handles "09:30" or just "09"
            // Split by ':' in case it's a full time string, then take the first part
            int hour = hourStr.split(':').first().toInt();
            qDebug() << "sale hour" << hour;

            if (m_filterTime == "Morning (6am-11am)") {
                matchTime = (hour >= 6 && hour < 11);
            } else if (m_filterTime == "Lunch (11am-3pm)") {
                matchTime = (hour >= 11 && hour < 15);
            } else if (m_filterTime == "Evening (3pm-10pm)") {
                matchTime = (hour >= 15 && hour <= 24);
            }
        }
        // Skip row if any filter fails
        if (!matchText || !matchMethod || !matchValue || !matchTime) continue;

        // --- 4. POPULATE UI MODEL ---
        QVariantMap row;
        row["order_id"] = orderId;
        row["waiter_id"] = waiter;
        row["gross_amount"] = gross;
        row["status"] = status.toUpper();
        row["sale_date"] = query.value("sale_date").toString();
        row["sale_hour"] = hourStr;
        row["signature"] = query.value("kra_receipt_number").toString();
        row["payment_id"] = payId;       // <--- NEW
        row["method"] = method;          // <--- NEW

        m_filteredData.append(row);

        // KPI Calculation (Only for filtered rows)
        m_totalGross += gross;
        if (row["signature"] == "MISSING") m_missingKraCount++;
    }

    // --- 5. CALCULATE VISUALIZATION MODELS ---

    // Maps for aggregation
    QMap<QString, double> dayMap;    // Date -> Revenue
    QMap<int, double> hourMap;      // Hour -> Revenue
    QMap<QString, QList<double>> staffMap; // Waiter -> List of ticket amounts

    // Ticket Size Buckets (e.g., 0-1k, 1k-5k, 5k-10k, 10k+)
    int bucketSmall = 0, bucketMed = 0, bucketLarge = 0, bucketPremium = 0;

    for (const QVariant &v : m_filteredData) {
        QVariantMap row = v.toMap();
        double gross = row["gross_amount"].toDouble();
        QString date = row["sale_date"].toString();
        int hour = row["sale_hour"].toString().toInt();
        QString waiter = row["waiter_id"].toString();

        // Trend aggregation
        dayMap[date] += gross;
        hourMap[hour] += gross;
        staffMap[waiter].append(gross);

        // Ticket size buckets
        if (gross < 1000) bucketSmall++;
        else if (gross < 5000) bucketMed++;
        else if (gross < 10000) bucketLarge++;
        else bucketPremium++;
    }

    // --- 6. FORMAT FOR QML ---

    // A. Daily Trend (Last 10 entries)
    m_dailyTrendModel.clear();
    QList<QString> days = dayMap.keys();
    std::sort(days.begin(), days.end()); // Ensure chronological order
    for (const QString &d : days.mid(qMax(0, days.size() - 10))) {
        QVariantMap entry;
        entry["label"] = QDate::fromString(d, "yyyy-MM-dd").toString("dd/MM");
        entry["value"] = dayMap[d];
        m_dailyTrendModel.append(entry);
    }

    // B. Hourly Heatmap (0-23 hours)
    m_hourlyHeatmapModel.clear();
    double maxHourRev = 0;
    for (double val : hourMap.values()) if (val > maxHourRev) maxHourRev = val;

    for (int h = 6; h <= 23; ++h) { // Typical restaurant hours
        QVariantMap entry;
        double total = hourMap.value(h, 0.0);
        entry["time"] = QString("%1:00").arg(h, 2, 10, QChar('0'));
        entry["total"] = total;
        entry["ratio"] = maxHourRev > 0 ? (total / maxHourRev) : 0;
        m_hourlyHeatmapModel.append(entry);
    }

    // C. Staff Performance
    m_staffPerformance.clear();
    for (auto it = staffMap.begin(); it != staffMap.end(); ++it) {
        double sum = 0;
        for (double g : it.value()) sum += g;
        QVariantMap entry;
        entry["name"] = it.key();
        entry["avg"] = it.value().isEmpty() ? 0 : (sum / it.value().size());
        m_staffPerformance.append(entry);
    }

    // D. Ticket Size Distribution
    m_ticketSizeModel.clear();
    m_ticketSizeModel << QVariantMap({{"label", "<1k"}, {"count", bucketSmall}});
    m_ticketSizeModel << QVariantMap({{"label", "1-5k"}, {"count", bucketMed}});
    m_ticketSizeModel << QVariantMap({{"label", "5-10k"}, {"count", bucketLarge}});
    m_ticketSizeModel << QVariantMap({{"label", ">10k"}, {"count", bucketPremium}});

    // ... [Previous aggregation logic] ...

    // --- 7. VISUALIZATION DIAGNOSTICS ---
    qDebug() << "======= Visualization Model Debug =======";

    qDebug() << "Daily Trend: " << m_dailyTrendModel.count() << "days";
    if (!m_dailyTrendModel.isEmpty())
        qDebug() << "  -> Latest Entry:" << m_dailyTrendModel.last().toMap()["label"]
                 << "Val:" << m_dailyTrendModel.last().toMap()["value"];

    qDebug() << "Hourly Heatmap:" << m_hourlyHeatmapModel.count() << "time slots";
    if (!m_hourlyHeatmapModel.isEmpty())
        qDebug() << "  -> Peak Hour Ratio Check (Index 0):" << m_hourlyHeatmapModel.at(0).toMap()["ratio"];

    qDebug() << "Staff Performance:" << m_staffPerformance.count() << "waiters";
    for(const QVariant &v : m_staffPerformance) {
        QVariantMap m = v.toMap();
        qDebug() << "  -> Waiter:" << m["name"] << "Avg Ticket:" << m["avg"];
    }

    qDebug() << "Ticket Size Buckets:";
    for(const QVariant &v : m_ticketSizeModel) {
        QVariantMap m = v.toMap();
        qDebug() << "  ->" << m["label"] << ":" << m["count"];
    }

    // qDebug() << "Top Products:" << m_topProductsModel.count() << "items";
    // if (!m_topProductsModel.isEmpty())
    //     qDebug() << "  -> #1 Product:" << m_topProductsModel.first().toMap()["name"]
    //              << "Qty:" << m_topProductsModel.first().toMap()["qty"];

    qDebug() << "==========================================";

    qDebug() << "Successfully processed" << rowsFound << "rows.";
    qDebug() << "--- END PROCESS DATA ---";
    emit dataProcessed();
}

void ReportController::fetchOrderItems(const QString &orderId) {
    m_selectedOrderItems.clear();

    QSqlQuery query;
    // Using R"(...)" prevents concatenation and whitespace bugs
    QString sql = R"(
        SELECT
            product_id, quantity, unit_price, tax_amount,
            tax_classification_code, service_state
        FROM order_item
        WHERE order_id = :orderId
    )";

    if (!query.prepare(sql)) {
        qCritical() << "PREPARE FAIL:" << query.lastError().text();
        return;
    }

    query.bindValue(":orderId", orderId);

    if (!query.exec()) {
        qCritical() << "SQL ERROR:" << query.lastError().text();
        qDebug() << "Full Query:" << query.executedQuery();
        qDebug() << "Bound Value:" << query.boundValue(":orderId");
        emit selectedOrderItemsChanged();
        return;
    }

    while (query.next()) {
        QVariantMap item;
        item["product"]  = query.value("product_id").toString();
        item["qty"]      = query.value("quantity").toDouble();
        item["price"]    = query.value("unit_price").toDouble();
        item["tax"]      = query.value("tax_amount").toDouble();
        item["tax_code"] = query.value("tax_classification_code").toString();
        item["state"]    = query.value("service_state").toString();
        m_selectedOrderItems.append(item);
    }

    emit selectedOrderItemsChanged();
}
