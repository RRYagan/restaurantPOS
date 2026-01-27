#ifndef PRODUCTFILTERPROXYMODEL_H
#define PRODUCTFILTERPROXYMODEL_H

#include "productmodel.h"

#include <QSortFilterProxyModel>

class ProductFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(int categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY filterChanged)
    Q_PROPERTY(int typeFilter READ typeFilter WRITE setTypeFilter NOTIFY filterChanged)

public:
    explicit ProductFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
        setFilterCaseSensitivity(Qt::CaseInsensitive);
        setDynamicSortFilter(true);
    }

    int categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(int id) {
        if (m_categoryFilter != id) {
            m_categoryFilter = id;
            invalidateFilter(); // Triggers a re-filter
            emit filterChanged();
        }
    }

    int typeFilter() const { return m_typeFilter; }
    void setTypeFilter(int id) {
        if (m_typeFilter != id) {
            m_typeFilter = id;
            invalidateFilter();
            emit filterChanged();
        }
    }

signals:
    void filterChanged();

protected:
    // This is the core logic that decides if a product stays or goes
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

        // Retrieve IDs from the source model (ProductModel)
        // Note: Replace "CategoryIdRole" with the actual role names defined in your ProductModel
        int rowCategory = sourceModel()->data(index, ProductModel::CategoryIdRole).toInt();
        int rowType = sourceModel()->data(index, ProductModel::ProductTypeIdRole).toInt();

        // If filter is -1 (or 0), we treat it as "Show All"
        bool categoryMatch = (m_categoryFilter <= 0 || rowCategory == m_categoryFilter);
        bool typeMatch = (m_typeFilter <= 0 || rowType == m_typeFilter);

        return categoryMatch && typeMatch;
    }

private:
    int m_categoryFilter{-1};
    int m_typeFilter{-1};
};

#endif
