#pragma once
#include <QObject>
#include <QStringList>
#include <QDate>
#include "event.h"

class EventRepository : public QObject {
    Q_OBJECT
public:
    explicit EventRepository(QObject *parent = nullptr);
    void loadDates();
    void loadPointsForDate(const QString &date);
    QList<Event> getEventsByDateRange(const QDate &from, const QDate &to);
    void clearLocalData();

signals:
    void datesLoaded(const QStringList &dates);
    void pointsLoaded(int count);
    void errorOccurred(const QString &message);
    void fetchProgress(int percent);

private slots:
    void onDataFetched(const QByteArray &data, const QUrl &url);
    void onFetchError(const QString &message);

private:
    void checkAndAddMissingDates(QStringList &dates);

    class DataFetcher *m_fetcher;
    class DatabaseManager *m_db;

    enum class FetchMode { None, Dates, Points };
    FetchMode m_mode = FetchMode::None;

    QString m_currentDate;
};