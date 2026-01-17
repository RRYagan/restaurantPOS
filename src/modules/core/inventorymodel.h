#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QDateTime>
#include <cstdint>
#include <QDebug>

struct InventoryItem {
    // 1. IDs and Strings
    QString id;
    QString name;
    QString packagingUnitId;
    QString quantityUnitId;
    double quantityPerPackage = 0.0;
    double quantityAvailable = 0.0;
    int32_t packagesAvailable = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
};

class InventoryModel : public QSqlTableModel {
    Q_OBJECT
    Q_PROPERTY(QString inventoryId READ inventoryId WRITE setInventoryId NOTIFY inventoryIdChanged)

public:
    explicit InventoryModel(QObject* parent = nullptr, const QSqlDatabase& db = QSqlDatabase());

    // Performance: Use smaller base type for Enum
    enum Roles : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        NameRole,
        PackagesAvailableRole,
        PackagingUnitNameRole,
        QuantityPerPackageRole,
        QuantityAvailableRole,
        QuantityUnitNameRole,
        CreatedAtRole,
        UpdatedAtRole
    };

    [[nodiscard]] auto inventoryId() const -> QString { return m_inventoryId; }

    auto setInventoryId(const QString& id) -> void {
        if (m_inventoryId != id) {
            qDebug() << "Controller: inventoryId changed from" << m_inventoryId << "to" << id;
            m_inventoryId = id;
            emit inventoryIdChanged();
        }
    }

    // --- READ ---
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    // --- CRUD METHODS ---
    auto createItem(const QVariantMap& data) -> bool;
    auto updateItem(const QVariantMap& data) -> bool;
    auto removeItem(const QString& id) -> bool;

    [[nodiscard]] auto inventoryAt(int row) const -> InventoryItem;
    [[nodiscard]] auto allItems() -> QList<InventoryItem>;

signals:
    auto inventoryIdChanged() -> void;

private:
    QString m_inventoryId = "";

    // Column Index Cache
    int m_idCol = -1;
    int m_nameCol = -1;
    int m_pkgAvailCol = -1;
    int m_pkgUnitCol = -1;
    int m_qpPkgCol = -1;
    int m_qtyAvailCol = -1;
    int m_qtyUnitCol = -1;
    int m_createdCol = -1;
    int m_updatedCol = -1;

    auto cacheIndices() -> void;
};

#endif // INVENTORYMODEL_H
