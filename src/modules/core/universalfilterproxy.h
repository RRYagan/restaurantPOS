#ifndef UNIVERSALFILTERPROXY_H
#define UNIVERSALFILTERPROXY_H

#include <QSortFilterProxyModel>
#include <QDate>
#include <QDateTime>

class UniversalFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT

    Q_PROPERTY(int filterId READ filterId WRITE setFilterId NOTIFY filterIdChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)
    Q_PROPERTY(QString categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)

public:
    enum FilterMode {
        All = 0,
        LowStock = 1,
        OutOfStock = 2
    };
    Q_ENUM(FilterMode)

    explicit UniversalFilterProxy(QObject *parent = nullptr);

    int filterId() const { return m_filterId; }
    void setFilterId(int id);

    QString searchString() const { return m_searchString; }
    void setSearchString(const QString &str);

    int filterMode() const { return m_filterMode; }
    void setFilterMode(int mode);

    QString categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(const QString &category);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

signals:
    // void filterChanged();
    void searchStringChanged();
    void filterModeChanged();
    void filterIdChanged();
    void categoryFilterChanged();

private:

    QString m_searchString;
    int m_filterMode = All;
    int m_filterId = -1;
    QString m_categoryFilter = "All";
};

#endif // UNIVERSALFILTERPROXY_H
