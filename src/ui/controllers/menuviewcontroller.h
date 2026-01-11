#ifndef MENUVIEWCONTROLLER_H
#define MENUVIEWCONTROLLER_H

#include <QObject>
#include "menutablemodel.h"
#include <QtQml/qqmlregistration.h>
#include "menucategorymodel.h"

class MenuViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(MenuTableModel* model READ model CONSTANT)
    Q_PROPERTY(MenuCategoryModel* categoryModel READ categoryModel NOTIFY categoriesChanged)

    Q_PROPERTY(QString currentCategory READ currentCategory WRITE setCurrentCategory NOTIFY currentCategoryChanged)


public:
    explicit MenuViewController(QObject* parent = nullptr);

    MenuTableModel* model() const { return m_model; }
    MenuCategoryModel* categoryModel() const { return m_categoryModel; }



    // --------------------
    // MenuItem CRUD
    // --------------------
    Q_INVOKABLE bool addMenuItem(const QString& name,
                                 const QString& description,
                                 const QString& categoryId,
                                 int basePriceCents,
                                 const QString& taxType,
                                 bool isAvailable,
                                 const QVector<MenuItemProduct>& recipe);

    Q_INVOKABLE bool updateMenuItem(const QString& id,
                                    const QString& name,
                                    const QString& description,
                                    const QString& categoryId,
                                    int basePriceCents,
                                    const QString& taxType,
                                    bool isAvailable,
                                    const QVector<MenuItemProduct>& recipe);

    Q_INVOKABLE bool deleteMenuItem(const QString& id);

    // --------------------
    // Categories
    // --------------------
    Q_INVOKABLE QVector<MenuCategory> getCategories() const;
    Q_INVOKABLE bool addCategory(const QString& id, const QString& name);
    Q_INVOKABLE bool updateCategory(const QString& id, const QString& name);
    Q_INVOKABLE bool deleteCategory(const QString& id);
    // filter
    QString currentCategory() const { return m_currentCategory; }
    // QVector<MenuCategory> categories
    void setCurrentCategory(const QString& category);

    // --------------------
    // Refresh
    // --------------------
    Q_INVOKABLE void reload();
signals:
    void currentCategoryChanged();
    void categoriesChanged();
private:
    // variable
    MenuTableModel* m_model;
    MenuCategoryModel* m_categoryModel;
    QVector<MenuCategory> m_categories;   // Cached categories

    QString m_currentCategory;

    // functions
    void loadCategories();
    void applyCategoryFilter();

};

#endif
