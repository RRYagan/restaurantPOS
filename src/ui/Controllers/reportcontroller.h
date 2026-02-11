#ifndef REPORTCONTROLLER_H
#define REPORTCONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QDateTime>
#include <QMap>
#include <QtQml/qqmlregistration.h>

class ReportController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // Filter Properties
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filtersChanged)
    Q_PROPERTY(QString filterMethod READ filterMethod WRITE setFilterMethod NOTIFY filtersChanged)
    Q_PROPERTY(QString filterTime READ filterTime WRITE setFilterTime NOTIFY filtersChanged)
    Q_PROPERTY(QString filterValue READ filterValue WRITE setFilterValue NOTIFY filtersChanged)

    // KPI Properties
    Q_PROPERTY(double todayRevenue READ todayRevenue NOTIFY dataProcessed)
    Q_PROPERTY(double avgDailyRevenue READ avgDailyRevenue NOTIFY dataProcessed)
    Q_PROPERTY(double revenueGrowth READ revenueGrowth NOTIFY dataProcessed)
    Q_PROPERTY(int missingKraCount READ missingKraCount NOTIFY dataProcessed)

    // Data Lists
    Q_PROPERTY(QVariantList filteredData READ filteredData NOTIFY dataProcessed)
    Q_PROPERTY(double totalGross READ totalGross NOTIFY dataProcessed)
    Q_PROPERTY(QVariantList selectedOrderItems READ selectedOrderItems NOTIFY selectedOrderItemsChanged)

    // Visualization Models
    Q_PROPERTY(QVariantList dailyTrendModel READ dailyTrendModel NOTIFY dataProcessed)
    Q_PROPERTY(QVariantList ticketSizeModel READ ticketSizeModel NOTIFY dataProcessed)
    Q_PROPERTY(QVariantList hourlyHeatmapModel READ hourlyHeatmapModel NOTIFY dataProcessed)
    Q_PROPERTY(QVariantList staffPerformance READ staffPerformance NOTIFY dataProcessed)

public:
    explicit ReportController(QObject *parent = nullptr);

    // Filter Accessors
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &t) { if(m_filterText != t) { m_filterText = t; processData(); } }

    QString filterMethod() const { return m_filterMethod; }
    void setFilterMethod(const QString &m) { if(m_filterMethod != m) { m_filterMethod = m; processData(); } }

    QString filterTime() const { return m_filterTime; }
    void setFilterTime(const QString &t) { if(m_filterTime != t) { m_filterTime = t; processData(); } }

    QString filterValue() const { return m_filterValue; }
    void setFilterValue(const QString &v) { if(m_filterValue != v) { m_filterValue = v; processData(); } }

    // Logic getters
    double todayRevenue() const { return m_todayRevenue; }
    double avgDailyRevenue() const { return m_avgDailyRevenue; }
    double revenueGrowth() const { return m_revenueGrowth; }
    int missingKraCount() const { return m_missingKraCount; }
    double totalGross() const { return m_totalGross; }
    QVariantList filteredData() const { return m_filteredData; }
    QVariantList dailyTrendModel() const { return m_dailyTrendModel; }
    QVariantList ticketSizeModel() const { return m_ticketSizeModel; }
    QVariantList hourlyHeatmapModel() const { return m_hourlyHeatmapModel; }
    QVariantList staffPerformance() const { return m_staffPerformance; }

    Q_INVOKABLE void refresh() { processData(); }

    QVariantList selectedOrderItems() const { return m_selectedOrderItems; }

    // Change the function to a Q_INVOKABLE so QML can call it
    Q_INVOKABLE void fetchOrderItems(const QString &orderId);

signals:
    void filtersChanged();
    void dataProcessed();
    void selectedOrderItemsChanged();

private:
    void processData();
    QDate m_startDate = QDate::currentDate().addDays(-30);
    QDate m_endDate = QDate::currentDate();

    // salesdetailes
    QVariantList m_selectedOrderItems;

    // Internal State
    QString m_filterText;
    QString m_filterMethod = "All Methods";
    QString m_filterTime = "Any Time";
    QString m_filterValue = "Any Value";

    double m_todayRevenue = 0;
    double m_avgDailyRevenue = 0;
    double m_revenueGrowth = 0;
    int m_missingKraCount = 0;
    double m_totalGross = 0;

    QVariantList m_filteredData;
    QVariantList m_dailyTrendModel;
    QVariantList m_ticketSizeModel;
    QVariantList m_hourlyHeatmapModel;
    QVariantList m_staffPerformance;

    void clearVisualModels();
};

#endif
