#include "menuview.h"
#include "databasemanager.h"

MenuView::MenuView(QObject *parent) : QObject(parent) {
    m_sourceModel = new BaseModel(this);
    m_proxy = new UniversalFilterProxy(this);

    // Set the data provider to fetch from DB
    m_sourceModel->setDataProvider([this]() {
        return m_model.getAllMenuItems();
    });

    // Link the proxy to the source
    m_proxy->setSourceModel(m_sourceModel);
}

void MenuView::refresh() {
    m_sourceModel->refresh();
}
QVariantList MenuView::getAllMenuItems() const {
    m_model.getAllMenuItems();
}
bool MenuView::addMenuItem(const QString &name, const QString &category, int price, const QString &icon) {
    if (m_model.addMenuItem(name, category, price, icon)) {
        refresh();
        return true;
    }
    return false;
}

bool MenuView::deleteItem(int itemId) {
    if (m_model.deleteMenuItem(itemId)) {
        refresh();
        return true;
    }
    return false;
}

bool MenuView::updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon) {
    // 1. Update the Database
    bool success = m_model.updateMenuItem(id, name, category, priceCents, icon);

    if (success) {
        // 2. Trigger the internal BaseModel to re-fetch data
        // This ensures the GridView in QML updates instantly
        m_sourceModel->refresh();
    }

    return success;
}

QStringList MenuView::categories() const {
    CategoryModel catModel;
    return catModel.fetchCategories();
}
