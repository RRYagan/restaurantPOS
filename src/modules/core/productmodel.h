#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <QString>
#include <QList>
#include <QSqlTableModel>

struct Product
{
    int localId = -1;
    QString id;
    QString kraUniqueItemCode;
    QString internalName;
    QString categoryCode;
    double  sellingPrice = 0.0;
    int     taxClassificationId = -1;
    int     measurementUnitId = -1;

    bool isValid() const
    {
        return !internalName.isEmpty()
        && measurementUnitId > 0;
    }
};

class ProductModel : public QSqlTableModel {
    Q_OBJECT


public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        KraCodeRole,
        NameRole,
        CategoryRole,
        PriceRole,
        TaxRole,
        UnitRole
    };

    explicit ProductModel(QObject* parent = nullptr,
                          QSqlDatabase db = QSqlDatabase());

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role) override;

    QHash<int, QByteArray> roleNames() const override;

    QList<Product> getAllProducts() const;
    bool addProduct(const Product &product);
    bool updateProduct(const Product& product);
    bool removeProduct(const QString& productId); // Changed from int to QString
    Product productAt(int row) const;
    //  Product getProductByKraCode(const QString &code);
    //  bool updatePrice(int id, double newPrice);
    //  QList<Product> getProductsByCategory(const QString &catCode);
};

#endif
