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

    // --- 1. Category Filter ---
    if (m_categoryFilter != "All" && !m_categoryFilter.isEmpty()) {
        if (item.value("category").toString() != m_categoryFilter) {
            return false; // Skip items that don't match the selected category
        }
    }


    // --- 1. Text Filter Logic ---
    bool matchesText = true;
    if (!m_searchString.isEmpty()) {
        QString name = item.value("name").toString();
        matchesText = name.contains(m_searchString, Qt::CaseInsensitive);
    }

    // ID filter
    if (m_filterId != -1) {
        int id = item.value("id").toInt(); // Changed from item_id to match menu schema
        if (id != m_filterId) return false;
    }

    // --- 2. Stock Filter Logic ---
    bool matchesStock = true;
    int qty = item.value("quantity").toInt();

    if (m_filterMode == LowStock) {
        /*matchesStock = (qty > 0 && qty <= 10);*/ // Adjust threshold as needed
        return qty > 0 && qty <= 10;
    } else if (m_filterMode == OutOfStock) {
        // matchesStock = (qty <= 0);
        return qty <=0;
    }

    // Row is shown only if BOTH filters pass
    // return matchesText && matchesStock;
    return true;
}
