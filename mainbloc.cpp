#include "mainbloc.h"
#include "eventrepository.h"
#include <QDebug>
#include <QDate>

MainBloc::MainBloc(QObject *parent) : QObject(parent) {
    m_repository = new EventRepository(this);

    connect(m_repository, &EventRepository::datesLoaded,
            this, &MainBloc::onDatesLoaded);
    connect(m_repository, &EventRepository::pointsLoaded,
            this, &MainBloc::onPointsLoaded);
    connect(m_repository, &EventRepository::errorOccurred,
            this, &MainBloc::onError);
}

MainBloc::~MainBloc() = default;

void MainBloc::initialize() {
    qDebug() << "[BLOC] === initialize() вызван ===";
    m_state.status = MainState::Status::LoadingDates;
    emitState();
    m_repository->loadDates();
}

void MainBloc::onDateSelected(const QString &date) {
    qDebug() << "[BLOC] === onDateSelected() вызван для:" << date << "===";
    m_state.currentDate = date;
    m_state.status = MainState::Status::LoadingPoints;
    emitState();
    m_repository->loadPointsForDate(date);
}

void MainBloc::onRefresh() {
    qDebug() << "[BLOC] === onRefresh() вызван ===";
    if (!m_state.currentDate.isEmpty())
        onDateSelected(m_state.currentDate);
}

void MainBloc::onClearData() {
    qDebug() << "[BLOC] === onClearData() вызван ===";
    m_repository->clearLocalData();
    m_state.events.clear();
    m_state.totalLoaded = 0;
    m_state.status = MainState::Status::Success;
    loadEventsForCurrentDate();
}

const MainState& MainBloc::state() const {
    return m_state;
}

void MainBloc::onDatesLoaded(const QStringList &dates) {
    qDebug() << "[BLOC] === onDatesLoaded() вызван с" << dates.size() << "датами ===";
    m_state.availableDates = dates;
    if (!dates.isEmpty()) {
        m_state.currentDate = dates.first();
        qDebug() << "[BLOC] Установлена текущая дата:" << m_state.currentDate;
        m_state.status = MainState::Status::LoadingPoints;
        emitState();
        qDebug() << "[BLOC] Вызываем loadPointsForDate для:" << m_state.currentDate;
        m_repository->loadPointsForDate(m_state.currentDate);
    } else {
        qDebug() << "[BLOC] Список дат пуст, устанавливаем ошибку";
        m_state.status = MainState::Status::Error;
        m_state.errorMessage = "Список дат пуст";
        emitState();
    }
}

void MainBloc::onPointsLoaded(int count) {
    qDebug() << "[BLOC] === onPointsLoaded() вызван с count =" << count << "===";
    m_state.totalLoaded += count;
    m_state.status = MainState::Status::Success;
    loadEventsForCurrentDate();
}

void MainBloc::onError(const QString &message) {
    qDebug() << "[BLOC] === onError() вызван:" << message << "===";
    m_state.status = MainState::Status::Error;
    m_state.errorMessage = message;
    emitState();
}

void MainBloc::emitState() {
    qDebug() << "[BLOC] emitState() - статус:" << static_cast<int>(m_state.status);
    emit stateChanged(m_state);
}

void MainBloc::loadEventsForCurrentDate() {
    if (!m_state.currentDate.isEmpty()) {
        QDate date = QDate::fromString(m_state.currentDate, "dd.MM.yyyy");
        m_state.events = m_repository->getEventsByDateRange(date, date);
        qDebug() << "[BLOC] Загружено событий из БД:" << m_state.events.size();
    }
    emitState();
}