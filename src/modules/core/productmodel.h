#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <QString>
#include <QList>
#include <QSqlTableModel>
#include <QHash>
#include <QByteArray>

struct Product
{
    int localId = -1;
    QString id;
    QString inventoryProductId;
    QString kraUniqueItemCode;
    QString internalName;
    QString categoryCode;   // Maps to product_category_id
    QString productTypeId;
    double  sellingPrice = 0.0;
    int     taxClassificationId = -1;
    int     quantity = 0;
    int     unitId = -1;

    bool isValid() const
    {
        return !internalName.isEmpty()
        && !inventoryProductId.isEmpty()
            && unitId  > 0;
    }
};

class ProductModel : public QSqlTableModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        InventoryIdRole,
        KraCodeRole,
        NameRole,
        CategoryRole,
        TypeRole,
        PriceRole,
        TaxRole,
        QuantityRole,
        UnitRole
    };

    explicit ProductModel(QObject* parent = nullptr,
                          QSqlDatabase db = QSqlDatabase());

    // Core Model Overrides
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // Database Operations
    QList<Product> getAllProducts() const;
    bool addProduct(const Product &product);
    bool updateProduct(const Product& product);
    bool removeProduct(const QString& productId);
    Product productAt(int row) const;

private:
    // Cached Column Indices
    int m_idCol;
    int m_invCol;
    int m_kraCol;
    int m_nameCol;
    int m_catCol;
    int m_typeCol;
    int m_priceCol;
    int m_taxCol;
    int m_qtyCol;
    int m_unitCol;
};

#endif // PRODUCTMODEL_H
