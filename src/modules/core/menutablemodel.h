#ifndef MENUTABLEMODEL_H
#define MENUTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSqlDatabase>
#include <QVector>
#include <QMap>

struct MenuCategory {
    QString id;
    QString name;
};

struct MenuItemProduct {
    QString menuItemId;
    QString productId;
    double quantity;
};
struct MenuItem {
    QString id;
    QString name;
    QString description;
    QString categoryId;      // FK to menu_categories
    QString categoryName;    // For displaying category in UI
    int basePriceCents;
    QString taxType;
    bool isAvailable;
    QVector<MenuItemProduct> recipe; // Products in this menu item
};

inline QDebug operator<<(QDebug debug, const MenuItem &item) {
    QDebugStateSaver saver(debug); // Ensures formatting resets after printing
    debug.nospace() << "MenuItem("
                    << "ID: " << item.id << ", "
                    << "Name: " << item.name << ", "
                    << "Price: " << item.basePriceCents << "c"
                    << ")";
    return debug;
}

class MenuTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MenuTableModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    enum MenuRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        PriceRole,
        CategoryRole,
        AvailableRole
    };
    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // CRUD API
    Q_INVOKABLE bool loadAll(); // fetch menu_items
    Q_INVOKABLE bool addMenuItem(const MenuItem& item, const QVector<MenuItemProduct>& recipe);
    Q_INVOKABLE bool updateMenuItem(const MenuItem& item, const QVector<MenuItemProduct>& recipe);
    Q_INVOKABLE bool deleteMenuItem(const QString& menuItemId);

    Q_INVOKABLE QVector<MenuCategory> getCategories() const;
    Q_INVOKABLE bool addCategory(const MenuCategory& category);
    Q_INVOKABLE bool updateCategory(const MenuCategory& category);
    Q_INVOKABLE bool deleteCategory(const QString& categoryId);
    void loadByCategoryName(const QString& categoryName);
    Q_INVOKABLE void setCategoryFilter(const QString &categoryId);
    Q_INVOKABLE QString categoryFilter() const { return m_categoryFilter; }


private:
    QSqlDatabase m_db;

    QVector<MenuItem> m_items;
    QMap<QString, QVector<MenuItemProduct>> m_recipes; // menuItemId -> recipe
    QString m_categoryFilter;

    bool updateRecipe(const QString& menuItemId, const QVector<MenuItemProduct>& recipe);
};

#endif // MENUTABLEMODEL_H
