#include "dataparser.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QTime>
#include <QtMath>
#include <QDebug>

QStringList DataParser::parseDates(const QByteArray &rawData) {
    QStringList dates;
    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (!doc.isArray()) {
        qDebug() << "[PARSER] dates.json is not an array";
        return dates;
    }

    QJsonArray array = doc.array();
    qDebug() << "[PARSER] Found" << array.size() << "dates in dates.json";

    for (const QJsonValue &val : array) {
        QString date = val.toString().trimmed();
        if (!date.isEmpty() && date != "null" && date.contains(".")) {
            dates.append(date);
        }
    }
    return dates;
}

QList<Event> DataParser::parsePoints(const QByteArray &rawData, const QString &date) {
    QList<Event> events;
    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (!doc.isArray()) {
        qWarning() << "[PARSER] Нет данных за дату:" << date;
        return events;  // Возвращаем пустой список вместо ошибки
    }

    QDate eventDate = QDate::fromString(date, "dd.MM.yyyy");
    if (!eventDate.isValid()) {
        qWarning() << "[PARSER] Невалидная дата:" << date;
        return events;
    }

    QJsonArray array = doc.array();
    qDebug() << "[PARSER] Обрабатываем" << array.size() << "точек за дату" << date;

    for (const QJsonValue &val : array) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        double lat = 0, lon = 0;
        QVariant latVal = obj.value("lat").toVariant();
        if (latVal.typeId() == QMetaType::QString)
            lat = latVal.toString().replace(",", ".").toDouble();
        else
            lat = obj.value("lat").toDouble();

        QVariant lonVal = obj.value("lng").toVariant();
        if (lonVal.typeId() == QMetaType::QString)
            lon = lonVal.toString().replace(",", ".").toDouble();
        else
            lon = obj.value("lng").toDouble();

        if (qIsNaN(lat) || qIsNaN(lon)) continue;

        QString text = obj.value("text").toString();
        QString icon = obj.value("icon").toString();
        if (icon.isEmpty() && text.isEmpty()) continue;

        Event e;
        e.title = obj.value("name").toString();
        if (e.title.isEmpty()) e.title = obj.value("fullName").toString();

        QStringList textParts = text.split("\n");
        e.description = textParts.isEmpty() ? "" : textParts[0];

        e.lat = lat;
        e.lon = lon;
        e.location = obj.value("fullName").toString();
        if (e.location.isEmpty()) e.location = e.title;
        e.category = icon;
        e.dateTime = eventDate.startOfDay();
        e.extId = QString("%1_%2_%3_%4")
                      .arg(date)
                      .arg(e.lat, 0, 'f', 6)
                      .arg(e.lon, 0, 'f', 6)
                      .arg(e.title);

        if (!e.title.isEmpty()) {
            events.append(e);
        }
    }

    qDebug() << "[PARSER] Найдено" << events.size() << "событий за дату" << date;
    return events;
}


QList<Event> DataParser::parseEvents(const QByteArray &rawData, const QString &date) {
    return parsePoints(rawData, date);
}