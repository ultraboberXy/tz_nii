#pragma once
#include <QString>
#include <QDateTime>

struct Event {
    int dbId = 0;
    QString extId;
    QString title;
    QString description;
    QDateTime dateTime;
    double lat = 0.0;
    double lon = 0.0;
    QString location;
    QString category;
    QString sourceUrl;
};