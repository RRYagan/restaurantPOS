#ifndef PRODUCTMODEL-R_H
#define PRODUCTMODEL-R_H

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVariantMap>
#include <vector>
#include "types.h"

class ProductModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        KraCodeRole,
        NameRole,
        CategoryRole,
        TypeRole,
        CurrencyRole,
        CountryOriginRole,
        PriceRole,
        PriceFormattedRole,
        TaxRole,
        TaxAmountRole,
        CategoryIdRole,
        ProductTypeIdRole,
        CompositionRole // Exposed as 'availableModifiers'
    };

    explicit ProductModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // --- PRODUCT CRUD (Matches ProductViewController requirements) ---
    auto addProduct(const QVariantMap &data) -> bool;
    auto updateProduct(const QVariantMap &data) -> bool; // Fixed signature
    auto removeProduct(const QString& productId) -> bool;

    // --- COMPOSITION CRUD (Matches ProductViewController requirements) ---
    auto addIngredient(const QString& productId, const QVariantMap& data) -> bool;
    auto updateIngredient(const QString& productId, const QVariantMap& data) -> bool;
    auto removeIngredient(const QString& productId, const QString& compositionId) -> bool;

    void refresh();
    [[nodiscard]] Product getProductById(const QString &id) const;

private:
    struct DeepProduct {
        Product product;
        QVariantList composition;
    };

    void loadData();
    void syncSingleProduct(const QString& productId);
    std::vector<DeepProduct> m_products;
};

#endif
