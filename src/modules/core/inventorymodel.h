#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>

struct InventoryItem {
    QString id;
    QString name;
    int packagesAvailable;
    int packagingUnitId;
    double quantityAvailable;
    int quantityUnitId;
    QString createdAt;
    QString updatedAt;

    // Helper: Map data from a QSqlRecord (Database -> Struct)
    // static InventoryItem fromRecord(const QSqlRecord& rec) {
    //     InventoryItem item;
    //     item.id = rec.value("id").toString();
    //     item.name = rec.value("name").toString();
    //     item.packagesAvailable = rec.value("packages_available").toInt();
    //     item.packagingUnitId = rec.value("packaging_unit_id").toInt();
    //     item.quantityAvailable = rec.value("quantity_available").toDouble();
    //     item.quantityUnitId = rec.value("quantity_unit_id").toInt();
    //     item.createdAt = rec.value("created_at").toString();
    //     item.updatedAt = rec.value("updated_at").toString();
    //     return item;
    // }

    // // Helper: Map data to a QVariantMap (Struct -> QML)
    // QVariantMap toMap() const {
    //     return {
    //         {"id", id},
    //         {"name", name},
    //         {"packagesAvailable", packagesAvailable},
    //         {"packagingUnitId", packagingUnitId},
    //         {"quantityAvailable", quantityAvailable},
    //         {"quantityUnitId", quantityUnitId},
    //         {"createdAt", createdAt},
    //         {"updatedAt", updatedAt}
    //     };
    // }
};

class InventoryModel : public QSqlTableModel {
    Q_OBJECT
    Q_PROPERTY(QString inventoryId READ inventoryId WRITE setInventoryId NOTIFY inventoryIdChanged)

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

    // Logic now uses the local m_inventoryId variable
    QString inventoryId() const { return m_inventoryId; }
    void setInventoryId(const QString& id) {
        qDebug() << "Controller: inventoryId changed from" << m_inventoryId << "to" << id;
        if (m_inventoryId != id) {
            m_inventoryId = id;
            emit inventoryIdChanged();
        }
    }
    // --- READ ---
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // --- CRUD METHODS ---
     bool createItem(const QVariantMap& data);
     bool updateItem(const QVariantMap& data);
     bool removeItem(const QString& id);
     InventoryItem inventoryAt(int row) const;
     QList<InventoryItem> allItems(); // Refresh/Clear filters

signals:
    void inventoryIdChanged();
private:
     QString m_inventoryId = "";
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
