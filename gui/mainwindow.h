#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

extern "C" {
    #include "src/core/engine.h"
    #include "src/core/workload.h"
    #include "src/policies/detector_wfg.h"
    #include "src/policies/detector_matrix.h"
    #include "src/policies/resolver_abort.h"
    #include "src/policies/resolver_rollback.h"
    #include "src/policies/prevention_banker.h"
    #include "src/policies/avoidance_nohold.h"
    #include "src/policies/avoidance_ordering.h"
    #include "src/policies/avoidance_preempt.h"
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadCSV();
    void onRunSimulation();
    void onNextStep();
    void onResetSimulation();

private:
    void setupUI();
    void updateTable();
    void updateLog(const QString& text);
    void setupEngine();

    Ui::MainWindow *ui;

    Engine    m_engine;
    EventList m_events;
    bool      m_loaded      = false;
    bool      m_engineReady = false;
    int       m_currentTime = 0;
    QTableWidget* m_allocTable;
    QTextEdit*    m_logView;
    QComboBox*    m_modeBox;
    QComboBox*    m_detectorBox;
    QLabel*       m_statusLabel;
    QPushButton*  m_btnStep;
    QPushButton*  m_btnRun;
    QPushButton*  m_btnReset;
};

#endif // MAINWINDOW_H
