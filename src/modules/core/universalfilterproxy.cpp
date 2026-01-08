#include "universalfilterproxy.h"
#include "basemodel.h"

UniversalFilterProxy::UniversalFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}


void UniversalFilterProxy::setSearchString(const QString& search) {
    m_searchString = search;
    // invalidateFilter(); // DEPRECATED
    beginFilterChange();
    endFilterChange();
    emit searchStringChanged();
}
// universalfilterproxy.cpp
void UniversalFilterProxy::setFilterMode(int mode) {
    m_filterMode = mode;
    beginFilterChange();
    endFilterChange();
    emit filterModeChanged();
}

void UniversalFilterProxy::setFilterId(int id) {
    m_filterId = id;
    beginFilterChange();
    endFilterChange();
    emit filterIdChanged();
}

void UniversalFilterProxy::setCategoryFilter(const QString& category) {
    m_categoryFilter = category;
    beginFilterChange();
    endFilterChange();
    emit categoryFilterChanged();
}

bool UniversalFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QVariantMap item = sourceModel()->data(index, BaseModel::DataRole).toMap();

    // 1. ID Filter (Critical for History Details)
    if (m_filterId != -1) {
        int id = item.value("item_id").toInt();
        if (id != m_filterId) {
            // qDebug() << "Rejecting ID:" << id << "Searching for:" << m_filterId;
            return false;
        }
        // qDebug() << "Match found for ID:" << id;
    }

    // 2. Category Filter
    if (m_categoryFilter != "All" && !m_categoryFilter.isEmpty()) {
        if (item.value("category").toString() != m_categoryFilter) return false;
    }

    // 3. Text Search Filter
    if (!m_searchString.isEmpty()) {
        QString name = item.value("name").toString();
        if (!name.contains(m_searchString, Qt::CaseInsensitive)) return false;
    }

    // 4. Stock Mode Filter
    int qty = item.value("quantity").toInt();
    if (m_filterMode == LowStock) {
        if (qty <= 0 || qty > 10) return false;
    } else if (m_filterMode == OutOfStock) {
        if (qty > 0) return false;
    }
    // If it passed all the "return false" checks above, it's a match!
    return true;
}
