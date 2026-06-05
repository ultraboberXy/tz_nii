#include "eventrepository.h"
#include "datafetcher.h"
#include "dataparser.h"
#include "databasemanager.h"
#include <QUrl>
#include <QDebug>
#include <algorithm>

static const QString BASE_URL =
    "https://cdndc.img.ria.ru/dc/kay-n/2022/SOP-content";

EventRepository::EventRepository(QObject *parent) : QObject(parent) {
    m_fetcher = new DataFetcher(this);
    m_db = new DatabaseManager("ria_events.db", this);

    connect(m_fetcher, &DataFetcher::finished,
            this, &EventRepository::onDataFetched);
    connect(m_fetcher, &DataFetcher::errorOccurred,
            this, &EventRepository::onFetchError);
    connect(m_fetcher, &DataFetcher::progress,
            this, &EventRepository::fetchProgress);

    m_db->initialize();
}

void EventRepository::loadDates() {
    qDebug() << "[REPO] === loadDates() вызван ===";
    m_mode = FetchMode::Dates;
    m_currentDate.clear();
    m_fetcher->fetchUrl(QUrl(BASE_URL + "/data/dates.json"));
}

void EventRepository::loadPointsForDate(const QString &date) {
    qDebug() << "[REPO] === loadPointsForDate() вызван для:" << date << "===";
    m_mode = FetchMode::Points;
    m_currentDate = date;
    m_fetcher->fetchUrl(
        QUrl(BASE_URL + "/data/points/data-" + date + ".json"));
}

QList<Event> EventRepository::getEventsByDateRange(const QDate &from, const QDate &to) {
    return m_db->getEventsByDateRange(from, to);
}

void EventRepository::clearLocalData() {
    m_db->clearAll();
}

void EventRepository::onDataFetched(const QByteArray &data, const QUrl &url) {
    qDebug() << "[REPO] === onDataFetched() вызван ===";
    qDebug() << "[REPO] URL:" << url.toString();
    qDebug() << "[REPO] Размер данных:" << data.size() << "байт";
    qDebug() << "[REPO] Текущий режим:" << static_cast<int>(m_mode);
    qDebug() << "[REPO] Текущая дата:" << m_currentDate;

    if (m_mode == FetchMode::Dates) {
        qDebug() << "[REPO] Обработка режима Dates";
        QStringList dates = DataParser::parseDates(data);
        qDebug() << "[REPO] Распарсено дат:" << dates.size();

        // Добавляем отсутствующие даты
        QDate today = QDate::currentDate();
        for (int i = 0; i < 7; ++i) {
            QString dateStr = today.addDays(-i).toString("dd.MM.yyyy");
            if (!dates.contains(dateStr)) {
                dates.prepend(dateStr);
                qDebug() << "[REPO] Добавлена отсутствующая дата:" << dateStr;
            }
        }

        // Сортировка
        std::sort(dates.begin(), dates.end(), [](const QString &a, const QString &b) {
            return QDate::fromString(a, "dd.MM.yyyy") > QDate::fromString(b, "dd.MM.yyyy");
        });

        m_mode = FetchMode::None;
        qDebug() << "[REPO] Эмитируем datesLoaded с" << dates.size() << "датами";
        emit datesLoaded(dates);

    } else if (m_mode == FetchMode::Points) {
        qDebug() << "[REPO] Обработка режима Points для даты:" << m_currentDate;
        QList<Event> events = DataParser::parsePoints(data, m_currentDate);
        qDebug() << "[REPO] Распарсено событий:" << events.size();

        m_db->saveEvents(events);
        qDebug() << "[REPO] События сохранены в БД";

        m_mode = FetchMode::None;
        qDebug() << "[REPO] Эмитируем pointsLoaded с count =" << events.size();
        emit pointsLoaded(events.size());
    } else {
        qWarning() << "[REPO] Неизвестный режим, игнорируем данные";
        m_mode = FetchMode::None;
    }
}

void EventRepository::onFetchError(const QString &message) {
    qWarning() << "[REPO] === onFetchError() вызван ===";
    qWarning() << "[REPO] Сообщение:" << message;
    qWarning() << "[REPO] Режим:" << static_cast<int>(m_mode);

    if (m_mode == FetchMode::Points) {
        qWarning() << "[REPO] Ошибка загрузки точек, эмитируем pointsLoaded(0)";
        m_mode = FetchMode::None;
        emit pointsLoaded(0);
    } else {
        m_mode = FetchMode::None;
        emit errorOccurred(message);
    }
}