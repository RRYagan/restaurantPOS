#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <QString>
#include <QList>
#include <QSqlTableModel>
#include <QHash>
#include <QByteArray>

struct Product {
    int localId = -1;
    QString id;
    // QString inventoryProductId;
    QString kraItemCode;
    QString internalProductName;
    QString productCategoryId;
    QString productTypeId;
    QString currencyCode;      // NEW: From schema
    QString countryCode; // NEW: From schema
    double defaultSellingPrice = 0.0;
    QString taxClassificationCode = "";
    double taxAmount = 0.0;  // NEW: From schema
    // double quantity = 0;
    // QString quantityUnitCode = "";

    bool isValid() const {
        return !id.isEmpty() &&
               // !inventoryProductId.isEmpty() &&
               !kraItemCode.isEmpty() &&
               !currencyCode.isEmpty() &&
               !countryCode.isEmpty();
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
        CurrencyRole,       // NEW
        CountryOriginRole,  // NEW
        PriceRole,
        TaxRole,
        TaxAmountRole,      // NEW
        // QuantityRole,
        // UnitRole
    };

    explicit ProductModel(QObject* parent = nullptr,
                          QSqlDatabase db = QSqlDatabase());

    static QString generateId(const QString &origin,QString const product_type,QString const pkg_unit) {
        // 1. Join the string parts
        // QString base = parts.join(separator);

        // 2. Format the counter: e.g., 5 -> "0000005"
        QString formattedCounter = QString("%1").arg(5, 0, 10, QChar('0'));

        // 3. Concatenate and return
        return origin + product_type + pkg_unit +formattedCounter;
    }

    // Core Model Overrides
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // Database Operations
    QList<Product> getAllProducts() const;
    bool addProduct(const QVariantMap &data);
    bool updateProduct(const QVariant &data);
    bool removeProduct(const QString& productId);
    Product productAt(int row) const;

private:
    // Cached Column Indices based on the updated schema
    int m_idCol;
    int m_invCol;
    int m_kraCol;
    int m_nameCol;
    int m_catCol;
    int m_typeCol;
    int m_currencyCol; // NEW: currency_id
    int m_countryCol;  // NEW: country_origin_id
    int m_priceCol;
    int m_taxCol;      // tax_classification_id
    int m_taxAmtCol;   // NEW: tax_amount
    // int m_qtyCol;
    // int m_unitCol;     // quantity_unit_id
};

#endif // PRODUCTMODEL_H
