#ifndef SQLITEMIGRATIONS_H
#define SQLITEMIGRATIONS_H
#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMap>
#include <QDebug>

namespace SqliteMigrations {

/* -------------------- Utilities -------------------- */

inline QString sha256(const QByteArray& data)
{
    return QCryptographicHash::hash(
               data, QCryptographicHash::Sha256).toHex();
}

inline bool execSql(QSqlDatabase& db, const QString& sql)
{
    QSqlQuery q(db);
    for (QString stmt : sql.split(';', Qt::SkipEmptyParts)) {
        stmt = stmt.trimmed();
        if (stmt.isEmpty())
            continue;

        if (!q.exec(stmt)) {
            qCritical() << q.lastError().text();
            return false;
        }
    }
    return true;
}

inline QByteArray loadFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return f.readAll();
}

/* -------------------- Locking (process-safe) -------------------- */

inline bool acquireLock(QSqlDatabase& db)
{
    QSqlQuery q(db);

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS migration_lock (
            id INTEGER PRIMARY KEY CHECK (id=1),
            locked INTEGER NOT NULL
        )
    )");

    q.exec("INSERT OR IGNORE INTO migration_lock VALUES (1, 0)");
    q.exec("UPDATE migration_lock SET locked=1 WHERE id=1 AND locked=0");

    return q.numRowsAffected() == 1;
}

inline void releaseLock(QSqlDatabase& db)
{
    QSqlQuery q(db);
    q.exec("UPDATE migration_lock SET locked=0 WHERE id=1");
}

/* -------------------- Schema tracking -------------------- */

inline bool ensureSchemaTable(QSqlDatabase& db)
{
    return execSql(db, R"(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version     INTEGER PRIMARY KEY,
            checksum    TEXT NOT NULL,
            applied_at  TEXT NOT NULL
        )
    )");
}

inline QMap<int, QString> appliedMigrations(QSqlDatabase& db)
{
    QMap<int, QString> map;
    QSqlQuery q(db);

    q.exec("SELECT version, checksum FROM schema_migrations");
    while (q.next())
        map[q.value(0).toInt()] = q.value(1).toString();

    return map;
}

/* -------------------- Migration Execution -------------------- */

inline bool applyUp(QSqlDatabase& db, int version, const QString& path)
{
    QByteArray sql = loadFile(path);
    if (sql.isEmpty())
        return false;

    QString checksum = sha256(sql);

    if (!db.transaction())
        return false;

    if (!execSql(db, QString::fromUtf8(sql))) {
        db.rollback();
        return false;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO schema_migrations (version, checksum, applied_at)
        VALUES (?, ?, ?)
    )");

    q.addBindValue(version);
    q.addBindValue(checksum);
    q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    if (!q.exec()) {
        db.rollback();
        return false;
    }

    return db.commit();
}

inline bool applyDown(QSqlDatabase& db, int version, const QString& path)
{
    QByteArray sql = loadFile(path);
    if (sql.isEmpty())
        return false;

    if (!db.transaction())
        return false;

    if (!execSql(db, QString::fromUtf8(sql))) {
        db.rollback();
        return false;
    }

    QSqlQuery q(db);
    q.prepare("DELETE FROM schema_migrations WHERE version=?");
    q.addBindValue(version);

    if (!q.exec()) {
        db.rollback();
        return false;
    }

    return db.commit();
}

/* -------------------- Public API -------------------- */

inline bool migrate(QSqlDatabase& db)
{
    if (!ensureSchemaTable(db))
        return false;

    if (!acquireLock(db)) {
        qCritical() << "Database is locked by another migration";
        return false;
    }

    QList<int> appliedThisRun;
    auto applied = appliedMigrations(db);

    QDir dir(":/migrations");
    QStringList files = dir.entryList(
        QStringList() << "*.up.sql", QDir::Name);

    for (const QString& file : files) {
        int version = file.left(3).toInt();
        QString upPath = ":/migrations/" + file;

        QByteArray sql = loadFile(upPath);
        QString checksum = sha256(sql);

        if (applied.contains(version)) {
            if (applied[version] != checksum) {
                qCritical() << "Checksum mismatch for migration" << version;
                goto rollback;
            }
            continue;
        }

        qInfo() << "Applying migration" << version;

        if (!applyUp(db, version, upPath))
            goto rollback;

        appliedThisRun.append(version);
    }

    releaseLock(db);
    return true;

rollback:
    qCritical() << "Migration failed. Rolling back.";

    for (auto it = appliedThisRun.crbegin();
         it != appliedThisRun.crend(); ++it) {

        QString downFile =
            QString("%1_*.down.sql").arg(*it, 3, 10, QChar('0'));

        QStringList matches = dir.entryList(QStringList() << downFile);
        if (matches.isEmpty())
            break;

        applyDown(db, *it, ":/migrations/" + matches.first());
    }

    releaseLock(db);
    return false;
}

} // namespace SqliteMigrations



#endif // SQLITEMIGRATIONS_H
