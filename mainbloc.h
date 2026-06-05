#pragma once
#include <QObject>
#include "mainstate.h"

class EventRepository;

class MainBloc : public QObject {
    Q_OBJECT
public:
    explicit MainBloc(QObject *parent = nullptr);
    ~MainBloc();

    void initialize();
    void onDateSelected(const QString &date);
    void onRefresh();
    void onClearData();

    const MainState& state() const;

signals:
    void stateChanged(const MainState &state);

private slots:
    void onDatesLoaded(const QStringList &dates);
    void onPointsLoaded(int count);
    void onError(const QString &message);

private:
    void emitState();
    void loadEventsForCurrentDate();

    MainState m_state;
    EventRepository *m_repository;
};