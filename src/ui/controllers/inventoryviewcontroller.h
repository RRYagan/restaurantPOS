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

public:
    explicit InventoryViewController(QObject *parent = nullptr);

    [[nodiscard]] auto inventoryModel() const -> InventoryModel* { return m_inventoryModel; }


    // Logic now uses the local m_inventoryId variable
    [[nodiscard]] auto inventoryId() const -> QString;
    void setInventoryId(const QString& id);


    // Updated: Now accepts a QVariantMap 'data'
    Q_INVOKABLE bool addStock(const QVariantMap &data);
    Q_INVOKABLE bool updateStock(const QVariantMap &data);
    Q_INVOKABLE bool deleteStock(const QString &id);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setCurrentInventoryProduct(const QString& id);

signals:
    void inventoryIdChanged();

private:
    InventoryModel* m_inventoryModel = nullptr;
    QString m_inventoryId; // Added this to store the state
};

#endif
