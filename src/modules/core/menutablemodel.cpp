#include "menutablemodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

MenuTableModel::MenuTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_db = DatabaseManager::instance().database();
    loadAll();
}

// -------------------- Model Interface --------------------

int MenuTableModel::rowCount(const QModelIndex&) const { return m_items.size(); }
int MenuTableModel::columnCount(const QModelIndex&) const { return 6; } // name, desc, category, price, tax, available

QHash<int, QByteArray> MenuTableModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[PriceRole] = "basePriceCents";
    roles[CategoryRole] = "categoryId";
    roles[AvailableRole] = "isAvailable";
    return roles;
}

QVariant MenuTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) return {};

    const MenuItem& item = m_items[index.row()];

    // Support both traditional DisplayRole (columns) and QML Role names
    if (role == Qt::DisplayRole || role == NameRole) return item.name;
    if (role == IdRole) return item.id;
    if (role == DescriptionRole) return item.description;
    if (role == PriceRole) return item.basePriceCents;
    if (role == CategoryRole) return item.categoryId;
    if (role == AvailableRole) return item.isAvailable;

    return {};
}

QVariant MenuTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch(section){
    case 0: return "Name";
    case 1: return "Description";
    case 2: return "Category";
    case 3: return "Price (cents)";
    case 4: return "Tax Type";
    case 5: return "Available";
    }
    return QVariant();
}

// -------------------- CRUD Implementation --------------------

bool MenuTableModel::loadAll()
{
    beginResetModel();
    m_items.clear();

    QSqlQuery query(DatabaseManager::instance().database());
    QString sql = R"(
        SELECT mi.id, mi.name, mi.description, mi.category_id, mc.name AS category_name,
               mi.base_price_cents, mi.tax_ty_cd, mi.is_available
        FROM menu_items AS mi
        LEFT JOIN menu_categories AS mc ON mi.category_id = mc.id
    )";

    if (!m_categoryFilter.isEmpty() && m_categoryFilter != "All") {
        sql += " WHERE mi.category_id = :catId";
    }

    query.prepare(sql);
    if (!m_categoryFilter.isEmpty() && m_categoryFilter != "All") {
        query.bindValue(":catId", m_categoryFilter);
    }

    if (!query.exec()) {
        qWarning() << "Failed to load menu items:" << query.lastError().text();
        return false;
    } else {
        while (query.next()) {
            MenuItem item;
            item.id = query.value("id").toString();
            item.name = query.value("name").toString();
            item.description = query.value("description").toString();
            item.categoryId = query.value("category_id").toString();
            item.categoryName = query.value("category_name").toString(); // cache for QML
            item.basePriceCents = query.value("base_price_cents").toInt();
            item.taxType = query.value("tax_ty_cd").toString();
            item.isAvailable = query.value("is_available").toBool();
            qDebug() << "menu items" << item.name << "price" << item.basePriceCents;
            m_items.append(item);

        }


    }

    endResetModel();
    return true;
}

void MenuTableModel::loadByCategoryName(const QString& categoryName) {
    if (categoryName.isEmpty() || categoryName == "All") {
        loadAll();
        return;
    }

    beginResetModel();
    m_items.clear();

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT mi.id, mi.name, mi.description, mi.base_price_cents, mi.tax_ty_cd, "
                  "mi.is_available, mc.name AS category_name "
                  "FROM menu_items mi "
                  "LEFT JOIN menu_categories mc ON mi.category_id = mc.id "
                  "WHERE mc.name = :categoryName");
    query.bindValue(":categoryName", categoryName);

    if (!query.exec()) {
        qCritical() << "Failed to load menu items by category:" << query.lastError().text();
        endResetModel();
        return;
    }

    while (query.next()) {
        MenuItem item;
        item.id = query.value("id").toString();
        item.name = query.value("name").toString();
        item.description = query.value("description").toString();
        item.basePriceCents = query.value("base_price_cents").toInt();
        item.taxType = query.value("tax_ty_cd").toString();
        item.isAvailable = query.value("is_available").toBool();
        item.categoryName = query.value("category_name").toString();

        m_items.append(item);
    }

    endResetModel();
}

bool MenuTableModel::addMenuItem(const MenuItem& item, const QVector<MenuItemProduct>& recipe)
{
    if(!m_db.transaction()) return false;

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO menu_items (id, name, description, category_id, base_price_cents, tax_ty_cd, is_available) "
              "VALUES (:id,:name,:desc,:cat,:price,:tax,:avail)");
    q.bindValue(":id", item.id);
    q.bindValue(":name", item.name);
    q.bindValue(":desc", item.description);
    q.bindValue(":cat", item.categoryId);
    q.bindValue(":price", item.basePriceCents);
    q.bindValue(":tax", item.taxType);
    q.bindValue(":avail", item.isAvailable);

    if(!q.exec()){
        qCritical() << "Add menu_item failed:" << q.lastError().text();
        m_db.rollback();
        return false;
    }

    if(!updateRecipe(item.id, recipe)){
        m_db.rollback();
        return false;
    }

    if(!m_db.commit()) return false;

    return loadAll();
}

bool MenuTableModel::updateMenuItem(const MenuItem& item, const QVector<MenuItemProduct>& recipe)
{
    if(!m_db.transaction()) return false;

    QSqlQuery q(m_db);
    q.prepare("UPDATE menu_items SET name=:name, description=:desc, category_id=:cat, "
              "base_price_cents=:price, tax_ty_cd=:tax, is_available=:avail WHERE id=:id");
    q.bindValue(":name", item.name);
    q.bindValue(":desc", item.description);
    q.bindValue(":cat", item.categoryId);
    q.bindValue(":price", item.basePriceCents);
    q.bindValue(":tax", item.taxType);
    q.bindValue(":avail", item.isAvailable);
    q.bindValue(":id", item.id);

    if(!q.exec()){
        qCritical() << "Update menu_item failed:" << q.lastError().text();
        m_db.rollback();
        return false;
    }

    if(!updateRecipe(item.id, recipe)){
        m_db.rollback();
        return false;
    }

    if(!m_db.commit()) return false;

    return loadAll();
}

bool MenuTableModel::deleteMenuItem(const QString& menuItemId)
{
    if(!m_db.transaction()) return false;

    QSqlQuery q(m_db);
    q.prepare("DELETE FROM menu_items WHERE id=:id");
    q.bindValue(":id", menuItemId);
    if(!q.exec()){
        qCritical() << "Delete menu_item failed:" << q.lastError().text();
        m_db.rollback();
        return false;
    }

    if(!m_db.commit()) return false;
    return loadAll();
}

bool MenuTableModel::updateRecipe(const QString& menuItemId, const QVector<MenuItemProduct>& recipe)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM menu_item_products WHERE menu_item_id=:id");
    q.bindValue(":id", menuItemId);
    if(!q.exec()) return false;

    for(const auto& mp : recipe){
        q.prepare("INSERT INTO menu_item_products (menu_item_id, product_id, quantity) VALUES (:mid,:pid,:qty)");
        q.bindValue(":mid", menuItemId);
        q.bindValue(":pid", mp.productId);
        q.bindValue(":qty", mp.quantity);
        if(!q.exec()) return false;
    }

    return true;
}

// -------------------- Categories --------------------

QVector<MenuCategory> MenuTableModel::getCategories() const
{
    QVector<MenuCategory> cats;
    QSqlQuery q(m_db);
    if(q.exec("SELECT id, name FROM menu_categories")){
        while(q.next()){
            cats.append({q.value(0).toString(), q.value(1).toString()});
        }
    }
    return cats;
}

bool MenuTableModel::addCategory(const MenuCategory& c)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO menu_categories (id, name) VALUES (:id,:name)");
    q.bindValue(":id", c.id);
    q.bindValue(":name", c.name);
    if(!q.exec()) return false;
    return true;
}

bool MenuTableModel::updateCategory(const MenuCategory& c)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE menu_categories SET name=:name WHERE id=:id");
    q.bindValue(":id", c.id);
    q.bindValue(":name", c.name);
    if(!q.exec()) return false;
    return true;
}

bool MenuTableModel::deleteCategory(const QString& categoryId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM menu_categories WHERE id=:id");
    q.bindValue(":id", categoryId);
    if(!q.exec()) return false;
    return true;
}
// menutablemodel.cpp
void MenuTableModel::setCategoryFilter(const QString &categoryId)
{
    if (m_categoryFilter == categoryId)
        return; // no change

    m_categoryFilter = categoryId;
    loadAll(); // reload items with the new filter
}
