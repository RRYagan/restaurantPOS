 #include "productviewcontroller.h"
#include <QDebug>
#include <QUuid>

ProductViewController::ProductViewController(QObject* parent)
    : QObject(parent)
{
    m_productModel = new ProductModel(this);
    m_compositionModel = new ProductCompositionModel(this);
}

bool ProductViewController::saveProduct(const QVariantMap& data) {
    int localFlag = data.value("localId", -1).toInt();
    QString id = data.value("id").toString();
     // 6. Persistence Logic
    if (localFlag == -1 && id.isEmpty()) {
        return m_productModel->addProduct(data);
    } else {
        return m_productModel->updateProduct(data);
    }
}

bool ProductViewController::saveIngredient(const QVariantMap& data) {
    ProductComposition c;
    int localFlag = data.value("localId", -1).toInt();

    QString id = data.value("id").toString();
    // c.productId = data.value("productId").toString();
    // c.ingredientProductId = data.value("ingredientProductId").toString();
    // c.quantity = data.value("quantity").toDouble();
    // c.unitId = data.value("unitId").toInt();

    if (localFlag == -1 && id.isEmpty()) {
        return m_compositionModel->addIngredient(data);
    } else {
        return m_compositionModel->updateIngredient(data);
    }
}

bool ProductViewController::removeProduct(const QString& productId) {
    return m_productModel->removeProduct(productId);
}

bool ProductViewController::removeIngredient(const QString& id) {
    return m_compositionModel->removeIngredient(id);
}

void ProductViewController::setCurrentProduct(const QString& productId) {
    m_compositionModel->setProductId(productId);
}
