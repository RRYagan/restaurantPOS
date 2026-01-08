#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include <QObject>
#include <QStringList>

class CategoryModel : public QObject {
    Q_OBJECT
public:
    explicit CategoryModel(QObject *parent = nullptr) : QObject(parent) {}

    QStringList fetchCategories();
    bool addCategory(const QString& name);
    bool updateCategory(const QString& oldName, const QString& newName);
    bool deleteCategory(const QString& categoryName);
};

#endif
