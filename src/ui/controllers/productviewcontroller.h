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

    [[nodiscard]] QString productId() const; // Only the declaration
    void setProductId(const QString& id);    // Only the declaration
    // clang-tidy off
    // UNITED FUNCTIONS
    Q_INVOKABLE bool saveProduct(const QVariantMap& data);
    Q_INVOKABLE bool saveIngredient(const QVariantMap& data);
    Q_INVOKABLE bool removeProduct(const QString& productId);
    Q_INVOKABLE bool removeIngredient(const QString& id);
    Q_INVOKABLE void setCurrentProduct(const QString& productId);
    // clang-tidy on

signals:
    void productIdChanged();

private:
    // This helper prevents the "not declared in scope" error
    // Product mapToProduct(const QVariantMap& map);

    ProductModel* m_productModel=nullptr;
    ProductCompositionModel* m_compositionModel;
};

#endif
