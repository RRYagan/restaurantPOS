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

    ProductModel* productModel() const { return m_productModel; }
    ProductCompositionModel* compositionModel() const { return m_compositionModel; }

    QString productId() const { return m_compositionModel->productId(); }
    void setProductId(const QString& id) {
        if (m_compositionModel->productId() != id) {
            m_compositionModel->setProductId(id);
            emit productIdChanged();
        }
    }
    // UNITED FUNCTIONS
    Q_INVOKABLE bool saveProduct(const QVariantMap& data);
    Q_INVOKABLE bool saveIngredient(const QVariantMap& data);
    Q_INVOKABLE bool removeProduct(const QString& productId);
    Q_INVOKABLE bool removeIngredient(const QString& id);
    Q_INVOKABLE void setCurrentProduct(const QString& productId);

signals:
    void productIdChanged();

private:
    // This helper prevents the "not declared in scope" error
    // Product mapToProduct(const QVariantMap& map);

    ProductModel* m_productModel;
    ProductCompositionModel* m_compositionModel;
};

#endif
