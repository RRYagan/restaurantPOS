#include "universalfilterproxy.h"
#include "basemodel.h"

UniversalFilterProxy::UniversalFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}


void UniversalFilterProxy::setSearchString(const QString &str) {
    if (m_searchString == str) return;
    m_searchString = str;
    // This tells the view to re-run the filter logic immediately
    invalidateFilter();
    emit searchStringChanged();
}
// universalfilterproxy.cpp
void UniversalFilterProxy::setFilterMode(int mode) {
    if (m_filterMode == mode) return;
    m_filterMode = mode;
    invalidateFilter(); // Re-run filter engine
    emit filterModeChanged();
}

void UniversalFilterProxy::setFilterId(int id)
{
    if (m_filterId != id) {
        m_filterId = id;
        emit filterIdChanged();
        invalidateFilter();
    }
}

void UniversalFilterProxy::setCategoryFilter(const QString &category) {
    if (m_categoryFilter == category) return;
    m_categoryFilter = category;
    invalidateFilter(); // Re-run the filter logic
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
