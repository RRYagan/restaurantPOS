#include "menumodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>

// menumodel.cpp
QVariantList MenuModel::getAllMenuItems() const {
    QVariantList list;
    QSqlDatabase db = DatabaseManager::instance().database();
    QSqlQuery query("SELECT id, name, category, base_price_cents, icon_source FROM menu_items", db);

    while (query.next()) {
        QVariantMap map;
        int itemId = query.value(0).toInt();
        map["id"] = itemId;
        map["name"] = query.value(1).toString();
        map["category"] = query.value(2).toString();
        map["price_cents"] = query.value(3).toLongLong();
        map["icon_source"] = query.value(4).toString();

        // Fetch Modifiers for this item
        QVariantList modifiers;
        QSqlQuery modQuery(db);
        modQuery.prepare("SELECT id, name, extra_price_cents FROM menu_item_modifiers WHERE menu_item_id = ?");
        modQuery.addBindValue(itemId);
        if (modQuery.exec()) {
            while (modQuery.next()) {
                QVariantMap mod;
                mod["id"] = modQuery.value(0).toInt();
                mod["name"] = modQuery.value(1).toString();
                mod["price_cents"] = modQuery.value(2).toLongLong();
                modifiers.append(mod);
            }
        }
        map["availableModifiers"] = modifiers;
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
