#ifndef LOOKUPMODEL_H
#define LOOKUPMODEL_H

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QVariant>

class LookupModel : public QSqlTableModel {
    Q_OBJECT

public:
    explicit LookupModel(const QString &tableName, QObject *parent = nullptr)
        : QSqlTableModel(parent) {
        setTable(tableName);
        setEditStrategy(QSqlTableModel::OnManualSubmit);
        select();
    }

    // CRITICAL: This allows QML to use column names as role names
    virtual QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        for (int i = 0; i < record().count(); ++i) {
            roles[Qt::UserRole + i + 1] = record().fieldName(i).toUtf8();
        }
        return roles;
    }

    // CRITICAL: Ensure QML can see data via the new roles
    virtual QVariant data(const QModelIndex &index, int role) const override {
        if (role > Qt::UserRole) {
            int column = role - Qt::UserRole - 1;
            QModelIndex modelIndex = this->index(index.row(), column);
            return QSqlTableModel::data(modelIndex, Qt::DisplayRole);
        }
        return QSqlTableModel::data(index, role);
    }

    // Helper to get the ID based on a Code (e.g., get ID for 'USD')
    int getIdByCode(const QString &codeValue, const QString &codeColumn = "code") {
        for (int i = 0; i < rowCount(); ++i) {
            if (record(i).value(codeColumn).toString() == codeValue) {
                return record(i).value("id").toInt();
            }
        }
        return -1;
    }

    // Helper to get the Display Name based on ID
    QString getNameById(int id, const QString &nameColumn = "name") {
        for (int i = 0; i < rowCount(); ++i) {
            if (record(i).value("id").toInt() == id) {
                return record(i).value(nameColumn).toString();
            }
        }
        return QString();
    }
    Q_INVOKABLE QVariant getValueById(const QVariant &lookupValue, const QString &columnName) {
        // Determine which column to search in based on the data type
        // If it's a string (like "A"), search "tax_type_code". If int, search "id".
        QString searchColumn = lookupValue.userType() == QMetaType::QString ? "tax_type_code" : "id";

        for (int i = 0; i < rowCount(); ++i) {
            if (record(i).value(searchColumn) == lookupValue) {
                return record(i).value(columnName);
            }
        }
        qWarning() << "getValueById: Value" << lookupValue << "not found in" << searchColumn;
        return 0.0;
    }
};

#endif // LOOKUPMODEL_H
