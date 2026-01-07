#include "menuview.h"
#include "databasemanager.h"

MenuView::MenuView(QObject *parent) : QObject(parent) {
    m_sourceModel = new BaseModel(this);
    m_proxy = new UniversalFilterProxy(this);

    // Set the data provider to fetch from DB
    m_sourceModel->setDataProvider([]() {
        return DatabaseManager::instance().getAllMenuItems();
    });

    // Link the proxy to the source
    m_proxy->setSourceModel(m_sourceModel);
}

void MenuView::refresh() {
    m_sourceModel->refresh();
}

bool MenuView::addMenuItem(const QString &name, const QString &category, int price, const QString &icon) {
    if (DatabaseManager::instance().addMenuItem(name, category, price, icon)) {
        refresh();
        return true;
    }
    return false;
}

bool MenuView::deleteItem(int itemId) {
    if (DatabaseManager::instance().deleteMenuItem(itemId)) {
        refresh();
        return true;
    }
    return false;
}
