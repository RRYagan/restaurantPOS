#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <functional>

class BaseModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum BaseRoles {
        DataRole = Qt::UserRole + 1
    };

    using DataProvider = std::function<QVariantList()>;

    explicit BaseModel(QObject *parent = nullptr);

    void setDataProvider(DataProvider provider);

    Q_INVOKABLE void refresh();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVariantList m_data;
    DataProvider m_provider;
};

#endif // BASEMODEL_H
