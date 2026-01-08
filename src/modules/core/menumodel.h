#ifndef MENUMODEL_H
#define MENUMODEL_H

#include "categorymodel.h"

#include <QObject>
#include <QVariantList>
#include <QStringList>

class MenuModel : public QObject {
    Q_OBJECT
public:
    explicit MenuModel(QObject *parent = nullptr) : QObject(parent) {}
    QVariantList getAllMenuItems() const;
    bool addMenuItem(const QString &name, const QString &category, int priceCents, const QString &icon);
    bool updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon);
    bool deleteMenuItem(int id);
    QStringList getAllCategories() {
        CategoryModel categoryModel;
        return categoryModel.fetchCategories();
    }
};
#endif
