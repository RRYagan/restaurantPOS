#include "menumodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>

QVariantList MenuModel::getAllMenuItems() const{
    QVariantList list;
    QSqlQuery query("SELECT id, name, category, base_price_cents, icon_source FROM menu_items", DatabaseManager::instance().database());
    while (query.next()) {
        QVariantMap map;
        map["id"] = query.value(0).toInt();
        map["name"] = query.value(1).toString();
        map["category"] = query.value(2).toString();
        map["price_cents"] = query.value(3).toLongLong();
        map["icon_source"] = query.value(4).toString();
        list.append(map);
    }
    return list;
}

// bool MenuModel::addMenuItem(const QString &name, const QString &category, int priceCents, const QString &icon) {
//     QSqlQuery q(DatabaseManager::instance().database());
//     q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");
//     q.addBindValue(name); q.addBindValue(category); q.addBindValue(priceCents); q.addBindValue(icon);
//     return q.exec();
// }

bool MenuModel::deleteMenuItem(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM menu_items WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}

bool MenuModel::addMenuItem(const QString &name, const QString &category, int priceCents, const QString &icon) {
    QSqlQuery q;
    q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");
    q.addBindValue(name);
    q.addBindValue(category);
    q.addBindValue(priceCents);
    q.addBindValue(icon);
    return q.exec();
}

// --- EDIT / UPDATE ITEM ---
bool MenuModel::updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon) {
    QSqlQuery q;
    q.prepare("UPDATE menu_items SET name = ?, category = ?, base_price_cents = ?, icon_source = ? WHERE id = ?");
    q.addBindValue(name);
    q.addBindValue(category);
    q.addBindValue(priceCents);
    q.addBindValue(icon);
    q.addBindValue(id);
    return q.exec();
}
