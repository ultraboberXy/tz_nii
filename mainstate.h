#pragma once
#include <QList>
#include <QStringList>
#include "event.h"

struct MainState {
    enum class Status { Idle, LoadingDates, LoadingPoints, Success, Error };
    Status status = Status::Idle;
    QList<Event> events;
    QStringList availableDates;
    QString currentDate;
    QString errorMessage;
    int totalLoaded = 0;
};