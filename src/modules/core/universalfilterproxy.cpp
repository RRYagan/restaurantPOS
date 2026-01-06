#include "universalfilterproxy.h"
#include "basemodel.h"

UniversalFilterProxy::UniversalFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void UniversalFilterProxy::setFilterDate(const QDate &d) {
    if (m_filterDate != d) { m_filterDate = d; invalidateFilter(); emit filterChanged(); }
}

void UniversalFilterProxy::setDateActive(bool a) {
    if (m_dateActive != a) { m_dateActive = a; invalidateFilter(); emit filterChanged(); }
}

void UniversalFilterProxy::setFilterTable(int n) {
    if (m_tableNumber != n) { m_tableNumber = n; invalidateFilter(); emit filterChanged(); }
}

void UniversalFilterProxy::setSearchString(const QString &s) {
    if (m_searchString != s) { m_searchString = s; invalidateFilter(); emit filterChanged(); }
}

bool UniversalFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // Use BaseModel::DataRole to get the variant map
    QVariantMap row = sourceModel()->data(index, BaseModel::DataRole).toMap();

    // 1. Hierarchy Level: Date
    if (m_dateActive && m_filterDate.isValid()) {
        QVariant dateVal = row.value("created_at");
        if (dateVal.isNull()) dateVal = row.value("date");

        if (dateVal.toDateTime().date() != m_filterDate) return false;
    }

    // 2. Hierarchy Level: Table Number
    if (m_tableNumber != -1) {
        if (row.value("table_number").toInt() != m_tableNumber) return false;
    }

    // 3. Hierarchy Level: Search (ID or Name)
    if (!m_searchString.isEmpty()) {
        QString id = row.value("id").toString();
        QString name = row.value("name").toString();

        bool matches = id.contains(m_searchString, Qt::CaseInsensitive) ||
                       name.contains(m_searchString, Qt::CaseInsensitive);

        if (!matches) return false;
    }

    return true;
}
