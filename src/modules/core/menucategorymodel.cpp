// #include "menucategorymodel.h"
// #include "databasemanager.h"
// #include <QSqlQuery>
// #include <QSqlError>
// #include <QDebug>

// QStringList CategoryModel::fetchCategories() {
//     QStringList categories;
//     QSqlQuery query("SELECT name FROM categories ORDER BY name ASC",
//                     DatabaseManager::instance().database());
//     while (query.next()) {
//         categories << query.value(0).toString();
//     }
//     return categories;
// }

// bool CategoryModel::addCategory(const QString& name) {
//     QSqlQuery query(DatabaseManager::instance().database());
//     query.prepare("INSERT INTO categories (name) VALUES (?)");
//     query.addBindValue(name);

//     if (!query.exec()) {
//         qWarning() << "Failed to add category:" << query.lastError().text();
//         return false;
//     }
//     return true;
// }

// bool CategoryModel::updateCategory(const QString& oldName, const QString& newName) {
//     QSqlDatabase db = DatabaseManager::instance().database();
//     if (!db.transaction()) return false;

//     QSqlQuery query(db);
//     // Update the category name in the categories table
//     query.prepare("UPDATE categories SET name = ? WHERE name = ?");
//     query.addBindValue(newName);
//     query.addBindValue(oldName);

//     if (!query.exec()) {
//         db.rollback();
//         return false;
//     }

//     // Update foreign keys in menu_items table
//     QSqlQuery itemQuery(db);
//     itemQuery.prepare("UPDATE menu_items SET category = ? WHERE category = ?");
//     itemQuery.addBindValue(newName);
//     itemQuery.addBindValue(oldName);

//     if (!itemQuery.exec()) {
//         db.rollback();
//         return false;
//     }

//     return db.commit();
// }

// bool CategoryModel::deleteCategory(const QString& categoryName) {
//     QSqlDatabase db = DatabaseManager::instance().database();
//     if (!db.transaction()) return false;

//     QSqlQuery query(db);

//     // First, delete items associated with this category
//     // (Or you could reassign them to 'Uncategorized')
//     query.prepare("DELETE FROM menu_items WHERE category = ?");
//     query.addBindValue(categoryName);
//     if (!query.exec()) {
//         db.rollback();
//         return false;
//     }

//     // Then delete the category itself
//     query.prepare("DELETE FROM categories WHERE name = ?");
//     query.addBindValue(categoryName);
//     if (!query.exec()) {
//         db.rollback();
//         return false;
//     }

//     return db.commit();
// }
