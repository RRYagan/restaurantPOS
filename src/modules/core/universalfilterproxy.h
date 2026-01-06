#ifndef UNIVERSALFILTERPROXY_H
#define UNIVERSALFILTERPROXY_H

#include <QSortFilterProxyModel>
#include <QDate>
#include <QDateTime>

class UniversalFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT

    Q_PROPERTY(QDate filterDate READ filterDate WRITE setFilterDate NOTIFY filterChanged)
    Q_PROPERTY(bool dateActive READ dateActive WRITE setDateActive NOTIFY filterChanged)
    Q_PROPERTY(int filterTable READ filterTable WRITE setFilterTable NOTIFY filterChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY filterChanged)

public:
    explicit UniversalFilterProxy(QObject *parent = nullptr);

    QDate filterDate() const { return m_filterDate; }
    void setFilterDate(const QDate &d);

    bool dateActive() const { return m_dateActive; }
    void setDateActive(bool a);

    // Renamed to match Q_PROPERTY READ name
    int filterTable() const { return m_tableNumber; }
    void setFilterTable(int n);

    QString searchString() const { return m_searchString; }
    void setSearchString(const QString &s);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

signals:
    void filterChanged();

private:
    QDate m_filterDate;
    bool m_dateActive = false;
    int m_tableNumber = -1;
    QString m_searchString;
};

#endif // UNIVERSALFILTERPROXY_H
