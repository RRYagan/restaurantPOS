#include "productviewcontroller.h"
#include <QDebug>

ProductViewController::ProductViewController(QObject* parent)
    : QObject(parent),
    m_productModel(new ProductModel(this)) // Uses the deep model
{
}

void ProductViewController::setProductId(const QString& id) {
    if (m_currentProductId != id) {
        m_currentProductId = id;
        emit productIdChanged();
    }
}

bool ProductViewController::saveProduct(const QVariantMap& data) {
    // If the data contains a valid ID, update; otherwise, add new
    qDebug() << "Controller received data:" << data; // Add this line
    QString id = data.value("id").toString();

    if (id.isEmpty()) {
        return m_productModel->addProduct(data);
    } else {
        return m_productModel->updateProduct(data);
    }
}

bool ProductViewController::saveIngredient(const QVariantMap& data) {
    if (m_currentProductId.isEmpty()) {
        qWarning() << "Cannot save ingredient: No current product ID set.";
        return false;
    }

    // compositionId is used to determine if we are updating an existing link
    QString compId = data.value("id").toString();

    if (compId.isEmpty()) {
        return m_productModel->addIngredient(m_currentProductId, data);
    } else {
        return m_productModel->updateIngredient(m_currentProductId, data);
    }
}

bool ProductViewController::removeIngredient(const QString& compositionId) {
    // Passes the request to the deep model to handle SQL and memory sync
    return m_productModel->removeIngredient(m_currentProductId, compositionId);
}

bool ProductViewController::removeProduct(const QString& productId) {
    return m_productModel->removeProduct(productId);
}

void ProductViewController::setCurrentProduct(const QString& productId) {
    setProductId(productId);
}
