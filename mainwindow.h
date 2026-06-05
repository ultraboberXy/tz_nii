#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QList>
#include "event.h"

class MainBloc;
struct MainState;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStateChanged(const MainState &state);
    void onDateSelected(int index);
    void onRefreshClicked();
    void onClearClicked();

private:
    void setupUi();
    void updateTable(const QList<Event> &events);

    MainBloc *m_bloc;
    QComboBox *m_dateCombo;
    QPushButton *m_refreshBtn;
    QPushButton *m_clearBtn;
    QTableWidget *m_table;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    bool m_updatingCombo = false;
};