#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>
#include "basemodel.h"
#include "universalfilterproxy.h"

class CategoryView : public QObject { // Changed from QAbstractListModel
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(UniversalFilterProxy* proxy READ proxy CONSTANT)

public:
    explicit CategoryView(QObject *parent = nullptr);

    UniversalFilterProxy* proxy() const { return m_proxy; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool addCategory(const QString &name);
    Q_INVOKABLE bool editCategory(const QString &oldName, const QString &newName);
    Q_INVOKABLE bool deleteCategory(const QString &name);

private:
    QVariantList getCategoryData(); // Provider for BaseModel

    BaseModel *m_internalModel;
    UniversalFilterProxy *m_proxy;
    QStringList m_categories;
};

#endif
