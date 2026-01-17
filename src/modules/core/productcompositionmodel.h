#ifndef PRODUCTCOMPOSITIONMODEL_H
#define PRODUCTCOMPOSITIONMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>
#include <cstdint>

struct ProductComposition {
    QString id;
    QString productId;
    QString ingredientProductId;
    QString unitId;
    double quantity = 0.0;

};

class ProductCompositionModel : public QSqlTableModel {
    Q_OBJECT
    Q_PROPERTY(QString productId READ productId WRITE setProductId NOTIFY productIdChanged)

public:
    explicit ProductCompositionModel(QObject* parent = nullptr, const QSqlDatabase& db = QSqlDatabase());

    enum Roles : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        ProductIdRole,
        IngredientIdRole,
        IngredientNameRole,
        QuantityRole,
        UnitRole
    };

    // --- READ ---
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    // --- PROPERTY ACCESSORS ---
    [[nodiscard]] auto productId() const -> QString;
    auto setProductId(const QString& id) -> void;

    // --- CRUD ---
    auto addIngredient(const QVariantMap& data) -> bool;
    auto updateIngredient(const QVariantMap& data) -> bool;
    auto removeIngredient(const QString& compositionId) -> bool;

    [[nodiscard]] auto compositionAt(int row) const -> ProductComposition;
    [[nodiscard]] auto getAllCompositions() const -> QList<ProductComposition>;

signals:
    void productIdChanged();

private:
    QString m_productId = "";

    // Member initializers to satisfy cppcoreguidelines-pro-type-member-init
    int m_idCol = -1;
    int m_productCol = -1;
    int m_ingredientCol = -1;
    int m_qtyCol = -1;
    int m_unitCol = -1;
};

#endif // PRODUCTCOMPOSITIONMODEL_H
