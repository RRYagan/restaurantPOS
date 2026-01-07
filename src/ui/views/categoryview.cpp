#include "categoryview.h"
#include "databasemanager.h"

CategoryView::CategoryView(QObject *parent) : QObject(parent) {
    m_internalModel = new BaseModel(this);
    m_proxy = new UniversalFilterProxy(this);

    // Link the provider: When refresh() is called, this lambda runs
    m_internalModel->setDataProvider([this]() { return getCategoryData(); });
    m_proxy->setSourceModel(m_internalModel);

    refresh();
}

QVariantList CategoryView::getCategoryData() {
    QVariantList list;
    for (const QString &category : m_categories) {
        QVariantMap map;
        map["name"] = category;
        list.append(map);
    }
    return list;
}

void CategoryView::refresh() {
    // Update the local cache from Database
    m_categories = DatabaseManager::instance().fetchCategories();

    // Trigger the internal model to reload and update the UI
    m_internalModel->refresh();
}

bool CategoryView::addCategory(const QString &name) {
    if (DatabaseManager::instance().addCategory(name)) {
        refresh();
        return true;
    }
    return false;
}

bool CategoryView::editCategory(const QString &oldName, const QString &newName) {
    if (newName.trimmed().isEmpty() || oldName == newName) return false;
    if (DatabaseManager::instance().updateCategory(oldName, newName)) {
        refresh();
        return true;
    }
    return false;
}

bool CategoryView::deleteCategory(const QString &name) {
    if (name == "All") return false;
    if (DatabaseManager::instance().deleteCategory(name)) {
        refresh();
        return true;
    }
    return false;
}
