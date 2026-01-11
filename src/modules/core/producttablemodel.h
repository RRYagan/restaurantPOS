#ifndef PRODUCTTABLEMODEL_H
#define PRODUCTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "product.h"
#include "databasemanager.h"

class ProductTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ProductTableModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Load products from DB
    Q_INVOKABLE bool loadProducts();

private:
    QVector<Product> m_products;
};

#endif // PRODUCTTABLEMODEL_H
