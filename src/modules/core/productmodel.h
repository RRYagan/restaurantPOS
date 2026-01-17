#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>
#include <cstdint>

// Assuming Product struct is defined here or included
struct Product {
    QString id;
    QString kraItemCode;
    QString internalProductName;
    QString productCategoryId;
    QString productTypeId;
    QString currencyCode;
    QString countryCode;
    double defaultSellingPrice = 0.0;
    QString taxClassificationCode;
    double taxAmount = 0.0;
};

class ProductModel : public QSqlTableModel {
    Q_OBJECT

public:
    explicit ProductModel(QObject* parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    // Rule of Five: Ensure consistent object lifecycle
    ~ProductModel() override = default;
    ProductModel(const ProductModel&) = delete;
    auto operator=(const ProductModel&) -> ProductModel& = delete;
    ProductModel(ProductModel&&) = delete;
    auto operator=(ProductModel&&) -> ProductModel& = delete;

    enum Roles : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        KraCodeRole,
        NameRole,
        CategoryRole,
        TypeRole,
        CurrencyRole,
        CountryOriginRole,
        PriceRole,
        TaxRole,
        TaxAmountRole
    };

    // --- READ ---
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    // --- WRITE ---
    auto setData(const QModelIndex& index, const QVariant& value, int role) -> bool override;

    // --- CRUD METHODS ---
    auto addProduct(const QVariantMap &data) -> bool;
    auto updateProduct(const QVariant &data) -> bool;
    auto removeProduct(const QString& productId) -> bool;

    [[nodiscard]] auto productAt(int row) const -> Product;
    [[nodiscard]] auto getAllProducts() const -> QList<Product>;

private:
    int m_idCol = -1;
    int m_kraCol = -1;
    int m_nameCol = -1;
    int m_catCol = -1;
    int m_typeCol = -1;
    int m_priceCol = -1;
    int m_taxCol = -1;
    int m_currencyCol = -1;
    int m_countryCol = -1;
    int m_taxAmtCol = -1;
};

#endif // PRODUCTMODEL_H
