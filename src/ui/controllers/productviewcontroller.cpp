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
    Product p;

    // Mapping payload from QML to Struct
    p.localId = data.value("localId", -1).toInt();
    p.id = data.value("id").toString();
    p.internalName = data.value("name").toString().trimmed();
    p.kraUniqueItemCode = data.value("kraCode").toString();
    p.sellingPrice = data.value("price").toDouble();

    // New fields from the updated SetupScreen UI
    p.inventoryProductId = data.value("inventoryId").toString();
    p.productTypeId = data.value("typeId").toString();
    p.quantity = data.value("quantity").toInt();

    // Categorization and Units
    p.categoryCode = data.value("categoryId").toString();
    p.taxClassificationId = data.value("taxId").toInt();
    p.unitId = data.value("unitId").toInt();

    // Validate using the logic in your Product struct
    if (!p.isValid()) {
        qWarning() << "Validation failed: Ensure Name, Inventory Link, and Unit are provided.";
        return false;
    }

    // Logic to determine Update vs Add
    bool isUpdate = !p.id.isEmpty();

    if (!isUpdate) {
        p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return m_productModel->addProduct(p);
    } else {
        return m_productModel->updateProduct(p);
    }
}

bool ProductViewController::saveIngredient(const QVariantMap& data) {
    ProductComposition c;
    int localFlag = data.value("localId", -1).toInt();

    c.id = data.value("uid").toString();
    c.productId = data.value("productId").toString();
    c.ingredientProductId = data.value("ingredientProductId").toString();
    c.quantity = data.value("quantity").toDouble();
    c.unitId = data.value("unitId").toInt();

    if (localFlag == -1 && c.id.isEmpty()) {
        c.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return m_compositionModel->addIngredient(c);
    } else {
        return m_compositionModel->updateIngredient(c);
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
