#include "basemodel.h"

BaseModel::BaseModel(QObject *parent) : QAbstractListModel(parent) {}

int BaseModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_data.size();
}

QVariant BaseModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_data.size()) return QVariant();
    if (role == DataRole) return m_data.at(index.row());
    return QVariant();
}

QHash<int, QByteArray> BaseModel::roleNames() const {
    return { {DataRole, "displayData"} };
}

void BaseModel::setDataProvider(DataProvider provider) {
    m_provider = provider;
    refresh();
}

void BaseModel::refresh() {
    if (!m_provider) return;
    beginResetModel();
    m_data = m_provider();
    endResetModel();
}
