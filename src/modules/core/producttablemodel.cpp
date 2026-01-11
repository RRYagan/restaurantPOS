#include "producttablemodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ProductTableModel::ProductTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ProductTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_products.size();
}

int ProductTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 10; // number of columns: id, name, code, classCode, typeCode, taxType, pkgUnit, qtyUnit, basePrice, isAvailable
}

QVariant ProductTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_products.size())
        return QVariant();

    const Product &p = m_products[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return p.id;
        case 1: return p.name;
        case 2: return p.code;
        case 3: return p.classCode;
        case 4: return p.typeCode;
        case 5: return p.taxType;
        case 6: return p.pkgUnit;
        case 7: return p.qtyUnit;
        case 8: return p.basePriceCents;
        case 9: return p.isAvailable;
        default: return QVariant();
        }
    }

    return QVariant();
}

QVariant ProductTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "ID";
        case 1: return "Name";
        case 2: return "Code";
        case 3: return "Class Code";
        case 4: return "Type Code";
        case 5: return "Tax Type";
        case 6: return "Package Unit";
        case 7: return "Quantity Unit";
        case 8: return "Base Price (cents)";
        case 9: return "Available";
        default: return QVariant();
        }
    }
    return QVariant();
}

bool ProductTableModel::loadProducts()
{
    beginResetModel();
    m_products.clear();

    QSqlQuery query(DatabaseManager::instance().database());
    if (!query.exec("SELECT id, item_nm, item_cd, item_cls_cd, item_ty_cd, tax_ty_cd, "
                    "pkg_unit_cd, qty_unit_cd, base_price_cents, is_available, user_id FROM products")) {
        qCritical() << "Failed to load products:" << query.lastError().text();
        endResetModel();
        return false;
    }

    while (query.next()) {
        Product p;
        p.id = query.value("id").toString();
        p.name = query.value("item_nm").toString();
        p.code = query.value("item_cd").toString();
        p.classCode = query.value("item_cls_cd").toString();
        p.typeCode = query.value("item_ty_cd").toString();
        p.taxType = query.value("tax_ty_cd").toString();
        p.pkgUnit = query.value("pkg_unit_cd").toString();
        p.qtyUnit = query.value("qty_unit_cd").toString();
        p.basePriceCents = query.value("base_price_cents").toInt();
        p.isAvailable = query.value("is_available").toBool();
        p.userId = query.value("user_id").toString();

        m_products.append(p);
    }

    endResetModel();
    return true;
}
