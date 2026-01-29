#ifndef PRODUCTVIEWCONTROLLER_H
#define PRODUCTVIEWCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include "productmodel.h"
#include "productcompositionmodel.h"
#include <QtQml/qqmlregistration.h>

class ProductViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ProductModel* productModel READ productModel CONSTANT)
    Q_PROPERTY(ProductCompositionModel* compositionModel READ compositionModel CONSTANT)
    Q_PROPERTY(QString productId READ productId WRITE setProductId NOTIFY productIdChanged)

public:
    explicit ProductViewController(QObject* parent = nullptr);

    [[nodiscard]] auto productModel() const -> ProductModel* { return m_productModel; }
    [[nodiscard]] auto compositionModel() const -> ProductCompositionModel*
    {
        return m_compositionModel;
    }

    [[nodiscard]] auto productId() const -> QString;
    void setProductId(const QString& id);

    /* QML Invokables: Trailing returns, [[nodiscard]] and auto not supported by moc */
    Q_INVOKABLE bool saveProduct(const QVariantMap& data);
    Q_INVOKABLE bool saveIngredient(const QVariantMap& data);
    Q_INVOKABLE bool removeProduct(const QString& productId);
    Q_INVOKABLE bool removeIngredient(const QString& id);
    Q_INVOKABLE void setCurrentProduct(const QString& productId);

signals:
    void productIdChanged();

private:
    ProductModel* m_productModel=nullptr;
    ProductCompositionModel* m_compositionModel=nullptr;
};

#endif
