#ifndef UNIVERSALFILTERPROXY_H
#define UNIVERSALFILTERPROXY_H

#include <QSortFilterProxyModel>
#include <QDate>
#include <QDateTime>

class UniversalFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT

    // Q_PROPERTY(QDate filterDate READ filterDate WRITE setFilterDate NOTIFY filterChanged)
    // Q_PROPERTY(bool dateActive READ dateActive WRITE setDateActive NOTIFY filterChanged)
    // Q_PROPERTY(int filterTable READ filterTable WRITE setFilterTable NOTIFY filterChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)

public:
    enum FilterMode {
        All = 0,
        LowStock = 1,
        OutOfStock = 2
    };
    Q_ENUM(FilterMode)

    explicit UniversalFilterProxy(QObject *parent = nullptr);



    QString searchString() const { return m_searchString; }
    void setSearchString(const QString &str);

    int filterMode() const { return m_filterMode; }
    void setFilterMode(int mode);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

signals:
    // void filterChanged();
    void searchStringChanged();
    void filterModeChanged();

private:

    QString m_searchString;
    int m_filterMode = All;
};

#endif // UNIVERSALFILTERPROXY_H
