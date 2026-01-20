 #include "productviewcontroller.h"
#include <QDebug>
#include <QUuid>

ProductViewController::ProductViewController(QObject* parent)
    : QObject(parent), m_productModel(new ProductModel(this)), m_compositionModel(new ProductCompositionModel(this)){}

auto ProductViewController::productId() const -> QString {
    return m_compositionModel->productId();
}

auto ProductViewController::setProductId(const QString& id) -> void {
    if (m_compositionModel->productId() != id) {
        m_compositionModel->setProductId(id);
        emit productIdChanged();
    }
}

auto ProductViewController::saveProduct(const QVariantMap& data) -> bool {
    // Create a mutable copy of the data to modify prices
    QVariantMap processedData = data;


    int localFlag = processedData.value("localId", -1).toInt();
    QString id = processedData.value("id").toString();

    if (localFlag == -1 && id.isEmpty()) {
        return m_productModel->addProduct(processedData);
    } else {
        return m_productModel->updateProduct(processedData);
    }
}
auto ProductViewController::saveIngredient(const QVariantMap& data) -> bool {
    ProductComposition c;
    int localFlag = data.value("localId", -1).toInt();

    QString id = data.value("id").toString();

    if (localFlag == -1 && id.isEmpty()) {
        return m_compositionModel->addIngredient(data);
    } else {
        return m_compositionModel->updateIngredient(data);
    }
}

auto ProductViewController::removeProduct(const QString& productId) -> bool {
    return m_productModel->removeProduct(productId);
}

auto ProductViewController::removeIngredient(const QString& id) -> bool {
    return m_compositionModel->removeIngredient(id);
}

auto ProductViewController::setCurrentProduct(const QString& productId) -> void {
    m_compositionModel->setProductId(productId);
}
