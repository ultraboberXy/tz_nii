#pragma once
#include <QObject>
#include <QList>
#include <QDate>
#include <QtSql/QSqlDatabase>
#include "event.h"

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(const QString &dbPath = "ria_events.db",
                             QObject *parent = nullptr);
    ~DatabaseManager();
    bool         initialize();
    bool         saveEvents(const QList<Event> &events);
    QList<Event> getEventsByDateRange(const QDate &from, const QDate &to);
    QList<Event> getAllEvents();
    int          totalCount();
    bool         clearAll();
    QString lastError() const { return m_lastError; }

private:
    bool createSchema();
    QSqlDatabase m_db;
    QString      m_dbPath;
    QString      m_lastError;
};