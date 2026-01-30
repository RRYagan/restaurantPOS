#ifndef GENERICFILTERPROXYMODEL_H
#define GENERICFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class GenericFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    // The values to filter for
    Q_PROPERTY(int categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY filterChanged)
    Q_PROPERTY(int typeFilter READ typeFilter WRITE setTypeFilter NOTIFY filterChanged)

    // The role names/IDs to look at in the source model
    Q_PROPERTY(int categoryRole READ categoryRole WRITE setCategoryRole NOTIFY rolesChanged)
    Q_PROPERTY(int typeRole READ typeRole WRITE setTypeRole NOTIFY rolesChanged)

public:
    explicit GenericFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
        setFilterCaseSensitivity(Qt::CaseInsensitive);
        setDynamicSortFilter(true);
    }

    // Filter Values
    int categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(int id) {
        if (m_categoryFilter != id) {
            m_categoryFilter = id;
            invalidateFilter();
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

    // Role Assignments
    int categoryRole() const { return m_categoryRole; }
    void setCategoryRole(int role) {
        if (m_categoryRole != role) {
            m_categoryRole = role;
            invalidateFilter();
            emit rolesChanged();
        }
    }

    int typeRole() const { return m_typeRole; }
    void setTypeRole(int role) {
        if (m_typeRole != role) {
            m_typeRole = role;
            invalidateFilter();
            emit rolesChanged();
        }
    }

signals:
    void filterChanged();
    void rolesChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

        // 1. Logic for Category Filter
        if (m_categoryRole != -1 && m_categoryFilter > 0) {
            int rowCategory = sourceModel()->data(index, m_categoryRole).toInt();
            if (rowCategory != m_categoryFilter) return false;
        }

        // 2. Logic for Type Filter
        if (m_typeRole != -1 && m_typeFilter > 0) {
            int rowType = sourceModel()->data(index, m_typeRole).toInt();
            if (rowType != m_typeFilter) return false;
        }

        QString pattern = filterRegularExpression().pattern();
        if (!pattern.isEmpty()) {
            QString rowData = sourceModel()->data(index, filterRole()).toString();

            // Perform exact match comparison
            if (rowData != pattern) return false;
        }

        return true;
    }

private:
    int m_categoryFilter{-1};
    int m_typeFilter{-1};

    int m_categoryRole{-1}; // Use -1 to indicate "ignore this filter"
    int m_typeRole{-1};
};

#endif
