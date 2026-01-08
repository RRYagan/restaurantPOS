#ifndef INVENTORYVIEW_H
#define INVENTORYVIEW_H

#include <QObject>
#include <inventorymodel.h>
#include <QtQml/qqmlregistration.h>
#include "basemodel.h"
#include "universalfilterproxy.h"

class InventoryView : public QObject {
    Q_OBJECT
    QML_ELEMENT
    // Expose the proxy so QML can bind to 'inventoryModel.proxy'
    Q_PROPERTY(UniversalFilterProxy* proxy READ proxy CONSTANT)
    Q_PROPERTY(UniversalFilterProxy* historyProxy READ historyProxy CONSTANT) // New property

public:
    explicit InventoryView(QObject *parent = nullptr);

    UniversalFilterProxy* proxy() const { return m_proxy; }
    UniversalFilterProxy* historyProxy() const { return m_historyProxy; }

    // Keep your CRUD methods
    Q_INVOKABLE bool addStock(const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool updateStock(int id, const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool deleteStock(int id);
    Q_INVOKABLE void refresh();

    // Q_INVOKABLE QVariantList getHistory(int itemId);
    // Q_INVOKABLE void loadHistory(int itemId);

private:
    BaseModel *m_sourceModel;
    UniversalFilterProxy *m_proxy;

    BaseModel *m_historySourceModel;       // Source for history logs
    UniversalFilterProxy *m_historyProxy; // Proxy for the history logs
    InventoryModel m_model;
};

#endif
