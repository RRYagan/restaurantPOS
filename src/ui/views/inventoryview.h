#ifndef INVENTORYVIEW_H
#define INVENTORYVIEW_H

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "basemodel.h"
#include "universalfilterproxy.h"

class InventoryView : public QObject {
    Q_OBJECT
    QML_ELEMENT
    // Expose the proxy so QML can bind to 'inventoryModel.proxy'
    Q_PROPERTY(UniversalFilterProxy* proxy READ proxy CONSTANT)

public:
    explicit InventoryView(QObject *parent = nullptr);

    UniversalFilterProxy* proxy() const { return m_proxy; }

    // Keep your CRUD methods
    Q_INVOKABLE bool addStock(const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool updateStock(int id, const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool deleteStock(int id);
    Q_INVOKABLE void refresh();

private:
    BaseModel *m_sourceModel;
    UniversalFilterProxy *m_proxy;
};

#endif
