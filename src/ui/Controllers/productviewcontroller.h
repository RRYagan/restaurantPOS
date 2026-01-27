#ifndef PRODUCTVIEWCONTROLLER_H
#define PRODUCTVIEWCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include <memory>
#include "productmodel.h"
#include <QtQml/qqmlregistration.h>

class ProductViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Provides the deep data source to QML
    Q_PROPERTY(ProductModel* productModel READ productModel CONSTANT)
    // Tracks which product is currently being edited for composition
    Q_PROPERTY(QString productId READ productId WRITE setProductId NOTIFY productIdChanged)

public:
    explicit ProductViewController(QObject* parent = nullptr);

    // Getters for QML properties
    ProductModel* productModel() const { return m_productModel; }
    QString productId() const { return m_currentProductId; }
    void setProductId(const QString& id);

    // QML Invokables for CRUD operations
    Q_INVOKABLE bool saveProduct(const QVariantMap& data);
    Q_INVOKABLE bool saveIngredient(const QVariantMap& data);
    Q_INVOKABLE bool removeProduct(const QString& productId);
    Q_INVOKABLE bool removeIngredient(const QString& compositionId);
    Q_INVOKABLE void setCurrentProduct(const QString& productId);

signals:
    void productIdChanged();

private:
    ProductModel* m_productModel = nullptr;
    QString m_currentProductId;
};

#endif // PRODUCTVIEWCONTROLLER_H
