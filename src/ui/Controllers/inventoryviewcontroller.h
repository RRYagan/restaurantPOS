#ifndef INVENTORYVIEWCONTROLLER_H
#define INVENTORYVIEWCONTROLLER_H

#include <QObject>
#include <inventorymodel.h>
#include <QtQml/qqmlregistration.h>

class InventoryViewController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(InventoryModel* inventoryModel READ inventoryModel CONSTANT)
    Q_PROPERTY(QString inventoryId READ inventoryId WRITE setInventoryId NOTIFY inventoryIdChanged)
    Q_PROPERTY(QString selectedUnitName READ selectedUnitName NOTIFY inventoryIdChanged)

public:
    explicit InventoryViewController(QObject *parent = nullptr);

    [[nodiscard]] auto inventoryModel() const -> InventoryModel* { return m_inventoryModel; }
    [[nodiscard]] auto inventoryId() const -> QString;
    void setInventoryId(const QString& id);
    /* Getter for the unit name */
    QString selectedUnitName() const {
        // Use the new lookup method from the model
        auto item = m_inventoryModel->getItemById(m_inventoryModel->inventoryId());
        return item.quantityUnitId; // This contains the unit name/ID from the DB
    }

    Q_INVOKABLE bool addStock(const QVariantMap &data);
    Q_INVOKABLE bool updateStock(const QVariantMap &data);
    Q_INVOKABLE bool deleteStock(const QString &id);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setCurrentInventoryProduct(const QString& id);
    Q_INVOKABLE QVariantMap getInventoryDetails(const QString& id) const;

signals:
    void inventoryIdChanged();

private:
    InventoryModel* m_inventoryModel = nullptr;
    QString m_inventoryId; /* Added this to store the state */
};

#endif
