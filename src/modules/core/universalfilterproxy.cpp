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

bool UniversalFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QVariantMap item = sourceModel()->data(index, BaseModel::DataRole).toMap();

    // --- 1. Text Filter Logic ---
    bool matchesText = true;
    if (!m_searchString.isEmpty()) {
        QString name = item.value("name").toString();
        matchesText = name.contains(m_searchString, Qt::CaseInsensitive);
    }

    // --- 2. Stock Filter Logic ---
    bool matchesStock = true;
    int qty = item.value("quantity").toInt();

    if (m_filterMode == LowStock) {
        matchesStock = (qty > 0 && qty <= 10); // Adjust threshold as needed
    } else if (m_filterMode == OutOfStock) {
        matchesStock = (qty <= 0);
    }

    // Row is shown only if BOTH filters pass
    return matchesText && matchesStock;
}
