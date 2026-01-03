#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QtQml/qqmlregistration.h>
#include <QDebug>
#include "databasemanager.h"

class CategoryModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
public:
    enum Roles { NameRole = Qt::UserRole + 1 };

    explicit CategoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // --- CRUD Logic ---
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool addCategory(const QString &name);
    Q_INVOKABLE bool editCategory(const QString &oldName, const QString &newName);
    Q_INVOKABLE bool deleteCategory(const QString &name);

private:
    QStringList m_categories;
};

#endif
