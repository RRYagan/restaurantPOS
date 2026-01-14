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

    InventoryModel* inventoryModel() const { return m_inventoryModel; }


    // Logic now uses the local m_inventoryId variable
    QString inventoryId() const { return m_inventoryId; }
    void setInventoryId(const QString& id) {
        qDebug() << "Controller: inventoryId changed from" << m_inventoryId << "to" << id;
        if (m_inventoryModel->inventoryId() != id) {
            m_inventoryModel->setInventoryId(id);
            emit inventoryIdChanged();
        }
    }


    Q_INVOKABLE bool addStock(const QVariantMap &data);
    Q_INVOKABLE bool updateStock(const QVariantMap &data);
    Q_INVOKABLE bool deleteStock(const QString &id);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setCurrentInventoryProduct(const QString& id);

signals:
    void inventoryIdChanged();

private:
    InventoryModel* m_inventoryModel;
    QString m_inventoryId; // Added this to store the state

    void debugInventoryData();
};

#endif
