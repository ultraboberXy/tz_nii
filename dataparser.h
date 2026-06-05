#pragma once
#include <QByteArray>
#include <QList>
#include <QStringList>
#include "event.h"

class DataParser {
public:
    static QStringList parseDates(const QByteArray &rawData);
    static QList<Event> parsePoints(const QByteArray &rawData, const QString &date);
    static QList<Event> parseEvents(const QByteArray &rawData, const QString &date);
};