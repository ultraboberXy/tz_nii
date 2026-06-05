#include "mainwindow.h"
#include "mainbloc.h"
#include "mainstate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_bloc = new MainBloc(this);
    connect(m_bloc, &MainBloc::stateChanged,
            this, &MainWindow::onStateChanged);
    setupUi();
    m_bloc->initialize();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("RIA Map Parser");
    resize(900, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel("Дата:"));

    m_dateCombo = new QComboBox();
    m_dateCombo->setMinimumWidth(150);
    controlsLayout->addWidget(m_dateCombo);

    m_refreshBtn = new QPushButton("Загрузить данные");
    controlsLayout->addWidget(m_refreshBtn);

    m_clearBtn = new QPushButton("Очистить БД");
    controlsLayout->addWidget(m_clearBtn);

    controlsLayout->addStretch();
    mainLayout->addLayout(controlsLayout);

    m_table = new QTableWidget();
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(
        {"Дата", "Название", "Локация", "Категория", "Координаты"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->setAlternatingRowColors(true);
    mainLayout->addWidget(m_table);

    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Готово");
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_progressBar);
    mainLayout->addLayout(statusLayout);

    connect(m_dateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDateSelected);
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);
    connect(m_clearBtn, &QPushButton::clicked,
            this, &MainWindow::onClearClicked);
}

void MainWindow::onStateChanged(const MainState &state) {
    m_updatingCombo = true;

    if (!state.availableDates.isEmpty()) {
        QStringList currentItems;
        for (int i = 0; i < m_dateCombo->count(); ++i)
            currentItems.append(m_dateCombo->itemText(i));

        if (currentItems != state.availableDates) {
            m_dateCombo->clear();
            m_dateCombo->addItems(state.availableDates);
        }
        int idx = state.availableDates.indexOf(state.currentDate);
        if (idx >= 0 && m_dateCombo->currentIndex() != idx)
            m_dateCombo->setCurrentIndex(idx);
    }

    m_updatingCombo = false;

    switch (state.status) {
    case MainState::Status::LoadingDates:
        m_statusLabel->setText("Загрузка списка дат...");
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0);
        m_refreshBtn->setEnabled(false);
        break;
    case MainState::Status::LoadingPoints:
        m_statusLabel->setText(
            QString("Загрузка точек за %1...").arg(state.currentDate));
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0);
        m_refreshBtn->setEnabled(false);
        break;
    case MainState::Status::Success:
        m_statusLabel->setText(
            QString("Дата: %1 | За дату: %2 | Всего в БД: %3")
                .arg(state.currentDate)
                .arg(state.events.size())
                .arg(state.totalLoaded));
        m_progressBar->setVisible(false);
        m_refreshBtn->setEnabled(true);
        updateTable(state.events);
        break;
    case MainState::Status::Error:
        m_statusLabel->setText("Ошибка: " + state.errorMessage);
        m_progressBar->setVisible(false);
        m_refreshBtn->setEnabled(true);
        QMessageBox::critical(this, "Ошибка", state.errorMessage);
        break;
    default:
        m_statusLabel->setText("Готово");
        m_progressBar->setVisible(false);
        m_refreshBtn->setEnabled(true);
        break;
    }
}

void MainWindow::onDateSelected(int index) {
    if (m_updatingCombo || index < 0) return;
    m_bloc->onDateSelected(m_dateCombo->itemText(index));
}

void MainWindow::onRefreshClicked() {
    m_bloc->onRefresh();
}

void MainWindow::onClearClicked() {
    m_bloc->onClearData();
}

void MainWindow::updateTable(const QList<Event> &events) {
    m_table->setRowCount(events.size());
    for (int i = 0; i < events.size(); ++i) {
        const Event &e = events[i];

        QTableWidgetItem *dateItem =
            new QTableWidgetItem(e.dateTime.toString("dd.MM.yyyy"));
        dateItem->setData(Qt::UserRole, e.dateTime);
        m_table->setItem(i, 0, dateItem);

        m_table->setItem(i, 1, new QTableWidgetItem(e.title));
        m_table->setItem(i, 2, new QTableWidgetItem(e.location));
        m_table->setItem(i, 3, new QTableWidgetItem(e.category));

        QString coords = QString("%1, %2")
                             .arg(e.lat, 0, 'f', 4)
                             .arg(e.lon, 0, 'f', 4);
        QTableWidgetItem *coordItem = new QTableWidgetItem(coords);
        coordItem->setData(Qt::UserRole, e.lat);
        m_table->setItem(i, 4, coordItem);
    }
}