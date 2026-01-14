#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>

class InventoryModel : public QSqlTableModel {
    Q_OBJECT
public:
    explicit InventoryModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase());

    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        PackagesAvailableRole,
        PackagingUnitIdRole,
        PackagingUnitNameRole,
        QuantityAvailableRole,
        QuantityUnitIdRole,
        QuantityUnitNameRole,
        CreatedAtRole,
        UpdatedAtRole
    };

    // --- READ ---
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // --- CRUD METHODS ---
    Q_INVOKABLE bool createItem(const QVariantMap& data);
    Q_INVOKABLE bool updateItem(const QVariantMap& data);
    Q_INVOKABLE bool removeItem(const QString& id);
    Q_INVOKABLE void allItems(); // Refresh/Clear filters

private:
    // Column Index Cache
    int m_idCol;
    int m_nameCol;
    int m_pkgAvailCol;
    int m_pkgUnitCol;
    int m_qtyAvailCol;
    int m_qtyUnitCol;
    int m_createdCol;
    int m_updatedCol;

    void cacheIndices();
};

#endif
