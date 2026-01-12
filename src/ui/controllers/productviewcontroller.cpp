#include "productviewcontroller.h"
#include <QDebug>
#include <QUuid>

ProductViewController::ProductViewController(QObject* parent)
    : QObject(parent)
{
    m_productModel = new ProductModel(this);
    m_compositionModel = new ProductCompositionModel(this);
}

// THE UNITED SAVE FUNCTION
bool ProductViewController::saveProduct(const QVariantMap& data) {
    Product p;
    // Map the localId to check for update status
    p.localId = data.value("localId", -1).toInt();
    p.id = data.value("id").toString();
    p.internalName = data.value("name").toString().trimmed();
    p.kraUniqueItemCode = data.value("kraCode").toString();
    p.sellingPrice = data.value("price").toDouble();
    p.categoryCode = data.value("categoryCode").toString();
    p.taxClassificationId = data.value("taxId").toInt();
    p.measurementUnitId = data.value("unitId").toInt();

    if (!p.isValid()) return false;

    // The logic check you requested
    bool isUpdate = (p.localId != -1);

    if (!isUpdate) {
        // NEW PRODUCT: Generate the DB ID (UUID)
        p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return m_productModel->addProduct(p);
    } else {
        // EXISTING PRODUCT: Update using the existing UUID
        return m_productModel->updateProduct(p);
    }
}

// Helper to map QVariantMap keys to Product struct members
// Product ProductViewController::mapToProduct(const QVariantMap& map)
// {
//     Product p;
//     // Map the keys used in your QML payload
//     p.id = map.value("id", -1).toInt();
//     p.internalName = map.value("name").toString().trimmed();
//     p.sellingPrice = map.value("price").toDouble();
//     p.kraUniqueItemCode = map.value("kraCode").toString();
//     p.categoryCode = map.value("categoryCode").toString();
//     p.taxClassificationId = map.value("taxId").toInt();
//     p.measurementUnitId = map.value("unitId").toInt();
//     return p;
// }

bool ProductViewController::saveIngredient(const QVariantMap& data) {
    ProductComposition c;
    int localFlag = data.value("localId", -1).toInt();

    c.id = data.value("uid").toString();
    c.productId = data.value("productId").toString(); // UUID of parent
    c.ingredientProductId = data.value("ingredientId").toString(); // UUID of ingredient
    c.quantity = data.value("quantity").toDouble();

    if (localFlag == -1) {
        c.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return m_compositionModel->addIngredient(c);
    } else {
        return m_compositionModel->updateIngredient(c);
    }
}

bool ProductViewController::removeProduct(const QString& productId)
{
    return m_productModel->removeProduct(productId);
}

bool ProductViewController::removeIngredient(const QString& id)
{
    return m_compositionModel->removeIngredient(id);
}

void ProductViewController::setCurrentProduct(const QString& productId)
{
    m_compositionModel->setProductId(productId);
}
