#pragma once

#include <QSqlTableModel>
#pragma once

struct ProductComposition
{
    int    localId = -1;
    QString  productId;
    QString id;
    QString name;
    QString    ingredientProductId;
    double quantity = 0.0;
    int    unitId = -1;

    bool isValid() const
    {
        return unitId  > 0
               && quantity > 0;
    }
};

class ProductCompositionModel : public QSqlTableModel
{
    Q_OBJECT
    Q_PROPERTY(QString productId READ productId WRITE setProductId NOTIFY productIdChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        ProductIdRole,
        IngredientIdRole,
        IngredientNameRole,
        QuantityRole,
        UnitRole
    };

    explicit ProductCompositionModel(QObject* parent = nullptr,
                                     QSqlDatabase db = QSqlDatabase());

    QString productId() const;
    void setProductId(const QString& id);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QList<ProductComposition> getAllCompositions() const;

    // CRUD
    bool addIngredient(const ProductComposition& c);
    bool updateIngredient(const ProductComposition& c);
    bool removeIngredient(const QString& compositionId);
    // DTO
    ProductComposition compositionAt(int row) const;

signals:
    void productIdChanged();

private:
    QString m_productId = "";

    //column indices
    int m_idCol;
    int m_productCol;
    int m_ingredientCol;
    int m_qtyCol;
    int m_unitCol;
};
