#include "menuviewcontroller.h"
#include "databasemanager.h"
#include <QDebug>
#include <QUuid>

MenuViewController::MenuViewController(QObject* parent)
    : QObject(parent),
    m_model(new MenuTableModel(this)), m_categoryModel(new MenuCategoryModel(this))
{
    // Initial load
    loadCategories();
    m_currentCategory = "All";
    m_model->loadAll();
    qDebug() << "Loaded item:" << m_model;
}

void MenuViewController::setCurrentCategory(const QString& category) {
    if (m_currentCategory != category) {
        m_currentCategory = category;
        applyCategoryFilter();
        emit currentCategoryChanged();
    }
}

// Applies the currentCategory filter to the MenuTableModel
void MenuViewController::applyCategoryFilter() {
    if (!m_model) return;

    if (m_currentCategory.isEmpty() || m_currentCategory == "All") {
        m_model->setCategoryFilter(QString()); // Show all
    } else {
        // Find category ID by name
        auto it = std::find_if(m_categories.begin(), m_categories.end(),
                               [this](const MenuCategory& c){ return c.name == m_currentCategory; });
        if (it != m_categories.end())
            m_model->setCategoryFilter(it->id);
        else
            m_model->setCategoryFilter(QString()); // fallback
    }
}

// Reload categories from DB and update category model
void MenuViewController::loadCategories() {
    m_categories.clear();
    m_categories.append(MenuCategory{ "All", "All" }); // Default category

    QVector<MenuCategory> dbCats = m_model->getCategories(); // Implement in model
    m_categories.append(dbCats);
    // 2. Update the dedicated QML model whenever data changes
    m_categoryModel->setCategories(m_categories);

    emit categoriesChanged();
}
// -------------------- Menu Item --------------------

bool MenuViewController::addMenuItem(const QString& name,
                                     const QString& description,
                                     const QString& categoryId,
                                     int basePriceCents,
                                     const QString& taxType,
                                     bool isAvailable,
                                     const QVector<MenuItemProduct>& recipe)
{
    MenuItem item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.name = name;
    item.description = description;
    item.categoryId = categoryId;
    item.basePriceCents = basePriceCents;
    item.taxType = taxType;
    item.isAvailable = isAvailable;

    return m_model->addMenuItem(item, recipe);
}

bool MenuViewController::updateMenuItem(const QString& id,
                                        const QString& name,
                                        const QString& description,
                                        const QString& categoryId,
                                        int basePriceCents,
                                        const QString& taxType,
                                        bool isAvailable,
                                        const QVector<MenuItemProduct>& recipe)
{
    MenuItem item;
    item.id = id;
    item.name = name;
    item.description = description;
    item.categoryId = categoryId;
    item.basePriceCents = basePriceCents;
    item.taxType = taxType;
    item.isAvailable = isAvailable;

    return m_model->updateMenuItem(item, recipe);
}

bool MenuViewController::deleteMenuItem(const QString& id)
{
    return m_model->deleteMenuItem(id);
}

// -------------------- Categories --------------------

QVector<MenuCategory> MenuViewController::getCategories() const
{
    return m_model->getCategories();
}

bool MenuViewController::addCategory(const QString& id, const QString& name)
{
    MenuCategory c{id, name};
    return m_model->addCategory(c);
}

bool MenuViewController::updateCategory(const QString& id, const QString& name)
{
    MenuCategory c{id, name};
    return m_model->updateCategory(c);
}

bool MenuViewController::deleteCategory(const QString& id)
{
    return m_model->deleteCategory(id);
}

// -------------------- Refresh --------------------

// --------------------
// Reload everything
// --------------------
void MenuViewController::reload()
{
    m_model->loadAll();
    loadCategories();
    applyCategoryFilter();
}
