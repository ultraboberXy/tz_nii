#include "databasemanager.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QDateTime>

DatabaseManager::DatabaseManager(const QString &dbPath, QObject *parent)
    : QObject(parent), m_dbPath(dbPath) {}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) m_db.close();
}

bool DatabaseManager::initialize() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "ria_conn");
    m_db.setDatabaseName(m_dbPath);
    if (!m_db.open()) { m_lastError = m_db.lastError().text(); return false; }
    return createSchema();
}

bool DatabaseManager::createSchema() {
    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL");
    bool ok = q.exec(R"(
        CREATE TABLE IF NOT EXISTS events (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            ext_id      TEXT    UNIQUE,
            title       TEXT    NOT NULL,
            description TEXT,
            event_date  TEXT,
            lat         REAL    DEFAULT 0,
            lon         REAL    DEFAULT 0,
            location    TEXT,
            category    TEXT,
            source_url  TEXT,
            fetched_at  TEXT
        )
    )");
    if (!ok) { m_lastError = q.lastError().text(); return false; }
    q.exec("CREATE INDEX IF NOT EXISTS idx_event_date ON events(event_date)");
    return true;
}

bool DatabaseManager::saveEvents(const QList<Event> &events) {
    if (events.isEmpty()) return true;
    m_db.transaction();
    QSqlQuery q(m_db);
    q.prepare(R"(
        INSERT OR IGNORE INTO events
        (ext_id, title, description, event_date, lat, lon,
         location, category, source_url, fetched_at)
        VALUES
        (:ext_id, :title, :desc, :date, :lat, :lon,
         :loc, :cat, :url, :fetched)
    )");
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    for (const Event &e : events) {
        q.bindValue(":ext_id",  e.extId.isEmpty() ? QVariant() : e.extId);
        q.bindValue(":title",   e.title);
        q.bindValue(":desc",    e.description);
        q.bindValue(":date",    e.dateTime.isValid() ? e.dateTime.toString(Qt::ISODate) : QVariant());
        q.bindValue(":lat",     e.lat);
        q.bindValue(":lon",     e.lon);
        q.bindValue(":loc",     e.location);
        q.bindValue(":cat",     e.category);
        q.bindValue(":url",     e.sourceUrl);
        q.bindValue(":fetched", now);
        if (!q.exec()) qWarning() << "Insert failed:" << q.lastError().text();
    }
    bool ok = m_db.commit();
    if (!ok) { m_db.rollback(); m_lastError = m_db.lastError().text(); }
    return ok;
}

QList<Event> DatabaseManager::getEventsByDateRange(const QDate &from, const QDate &to) {
    QSqlQuery q(m_db);
    q.prepare(R"(
        SELECT id, ext_id, title, description, event_date,
               lat, lon, location, category, source_url
        FROM events WHERE date(event_date) BETWEEN :from AND :to
        ORDER BY event_date ASC
    )");
    q.bindValue(":from", from.toString(Qt::ISODate));
    q.bindValue(":to",   to.toString(Qt::ISODate));
    QList<Event> result;
    if (q.exec()) {
        while (q.next()) {
            Event e;
            e.dbId        = q.value(0).toInt();
            e.extId       = q.value(1).toString();
            e.title       = q.value(2).toString();
            e.description = q.value(3).toString();
            e.dateTime    = QDateTime::fromString(q.value(4).toString(), Qt::ISODate);
            e.lat         = q.value(5).toDouble();
            e.lon         = q.value(6).toDouble();
            e.location    = q.value(7).toString();
            e.category    = q.value(8).toString();
            e.sourceUrl   = q.value(9).toString();
            result.append(e);
        }
    } else { m_lastError = q.lastError().text(); }
    return result;
}

QList<Event> DatabaseManager::getAllEvents() {
    QSqlQuery q(m_db);
    q.prepare(R"(
        SELECT id, ext_id, title, description, event_date,
               lat, lon, location, category, source_url
        FROM events ORDER BY event_date DESC
    )");
    QList<Event> result;
    if (q.exec()) {
        while (q.next()) {
            Event e;
            e.dbId        = q.value(0).toInt();
            e.extId       = q.value(1).toString();
            e.title       = q.value(2).toString();
            e.description = q.value(3).toString();
            e.dateTime    = QDateTime::fromString(q.value(4).toString(), Qt::ISODate);
            e.lat         = q.value(5).toDouble();
            e.lon         = q.value(6).toDouble();
            e.location    = q.value(7).toString();
            e.category    = q.value(8).toString();
            e.sourceUrl   = q.value(9).toString();
            result.append(e);
        }
    }
    return result;
}

int DatabaseManager::totalCount() {
    QSqlQuery q(m_db);
    q.exec("SELECT COUNT(*) FROM events");
    return q.next() ? q.value(0).toInt() : 0;
}

bool DatabaseManager::clearAll() {
    QSqlQuery q(m_db);
    bool ok = q.exec("DELETE FROM events");
    if (!ok) m_lastError = q.lastError().text();
    return ok;
}