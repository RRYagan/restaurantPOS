#include "productviewcontroller.h"
#include <QDebug>

ProductViewController::ProductViewController(QObject* parent)
    : QObject(parent)
{
    m_model = new ProductTableModel(this);
}

void ProductViewController::loadProducts()
{
    if (!m_model->loadProducts()) {
        qWarning() << "Failed to load products from database.";
    }
}

void ProductViewController::refresh()
{
    // Clear and reload
    loadProducts();
    qDebug() << "Product list refreshed.";
}
