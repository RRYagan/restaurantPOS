#ifndef PRODUCTVIEWCONTROLLER_H
#define PRODUCTVIEWCONTROLLER_H

#include <QObject>
#include "producttablemodel.h"
#include <QtQml/qqmlregistration.h>

class ProductViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ProductTableModel* productModel READ productModel CONSTANT)

public:
    explicit ProductViewController(QObject* parent = nullptr);

    ProductTableModel* productModel() const { return m_model; }

    // QML callable functions
    Q_INVOKABLE void loadProducts();
    Q_INVOKABLE void refresh();

private:
    ProductTableModel* m_model;
};

#endif // PRODUCTVIEWCONTROLLER_H
