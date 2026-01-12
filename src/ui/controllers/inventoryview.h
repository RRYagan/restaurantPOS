#ifndef INVENTORYVIEW_H
#define INVENTORYVIEW_H

#include <QObject>
#include <inventorymodel.h>
#include <QtQml/qqmlregistration.h>


class InventoryView : public QObject {
    Q_OBJECT
    QML_ELEMENT


public:
    explicit InventoryView(QObject *parent = nullptr);



    // Keep your CRUD methods
    Q_INVOKABLE bool addStock(const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool updateStock(int id, const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool deleteStock(int id);
    Q_INVOKABLE void refresh();

    // Q_INVOKABLE QVariantList getHistory(int itemId);
    // Q_INVOKABLE void loadHistory(int itemId);

private:


    InventoryModel m_model;
};

#endif
