#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
    engine_init(&m_engine);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    setWindowTitle("Deadlock Simulator");
    resize(1000, 650);

    // --- Buttons ---
    QPushButton* btnLoad = new QPushButton("📂 Load CSV");
    m_btnRun   = new QPushButton("▶ Run All");
    m_btnStep  = new QPushButton("⏭ Next Step");
    m_btnReset = new QPushButton("🔄 Reset");

    // Step und Reset am Anfang deaktiviert
    m_btnStep->setEnabled(false);
    m_btnRun->setEnabled(false);
    m_btnReset->setEnabled(false);

    // --- Dropdowns ---
    m_modeBox = new QComboBox();
    m_modeBox->addItems({"detect_abort","detect_rollback","banker",
                         "avoid_nohold","avoid_order","avoid_preempt"});

    m_detectorBox = new QComboBox();
    m_detectorBox->addItems({"matrix","wfg"});

    // --- Top Bar ---
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->addWidget(btnLoad);
    topBar->addWidget(new QLabel("Mode:"));
    topBar->addWidget(m_modeBox);
    topBar->addWidget(new QLabel("Detector:"));
    topBar->addWidget(m_detectorBox);
    topBar->addWidget(m_btnRun);
    topBar->addWidget(m_btnStep);
    topBar->addWidget(m_btnReset);
    topBar->addStretch();

    // --- Alloc Table ---
    m_allocTable = new QTableWidget(this);
    QGroupBox* tableBox = new QGroupBox("Allocation Matrix (Prozess × Ressource)");
    QVBoxLayout* tableLayout = new QVBoxLayout();
    tableLayout->addWidget(m_allocTable);
    tableBox->setLayout(tableLayout);

    // --- Log ---
    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(180);
    QGroupBox* logBox = new QGroupBox("Simulation Log");
    QVBoxLayout* logLayout = new QVBoxLayout();
    logLayout->addWidget(m_logView);
    logBox->setLayout(logLayout);

    // --- Status ---
    m_statusLabel = new QLabel("Bereit. Bitte CSV laden.");

    // --- Haupt-Layout ---
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addLayout(topBar);
    mainLayout->addWidget(tableBox);
    mainLayout->addWidget(logBox);
    mainLayout->addWidget(m_statusLabel);
    central->setLayout(mainLayout);

    // --- Signals ---
    connect(btnLoad,   &QPushButton::clicked, this, &MainWindow::onLoadCSV);
    connect(m_btnRun,  &QPushButton::clicked, this, &MainWindow::onRunSimulation);
    connect(m_btnStep, &QPushButton::clicked, this, &MainWindow::onNextStep);
    connect(m_btnReset,&QPushButton::clicked, this, &MainWindow::onResetSimulation);
}

// ─── Engine konfigurieren (ausgelagert, wird von Run + Step genutzt) ──────────
void MainWindow::setupEngine() {
    engine_init(&m_engine);

    if (m_detectorBox->currentText() == "wfg")
        engine_set_detector(&m_engine, detector_wfg());
    else
        engine_set_detector(&m_engine, detector_matrix());

    QString mode = m_modeBox->currentText();
    if      (mode == "detect_abort")    engine_set_resolver(&m_engine, resolver_abort());
    else if (mode == "detect_rollback") engine_set_resolver(&m_engine, resolver_rollback());
    else if (mode == "banker")          engine_set_policy(&m_engine, policy_banker());
    else if (mode == "avoid_nohold")    engine_set_policy(&m_engine, policy_avoid_nohold());
    else if (mode == "avoid_order")     engine_set_policy(&m_engine, policy_avoid_ordering());
    else if (mode == "avoid_preempt")   engine_set_policy(&m_engine, policy_avoid_preempt());

    engine_load_events(&m_engine, &m_events);
    m_currentTime  = 0;
    m_engineReady  = true;
}

// ─── CSV laden ────────────────────────────────────────────────────────────────
void MainWindow::onLoadCSV() {
    QString path = QFileDialog::getOpenFileName(
        this, "CSV laden", "", "CSV Dateien (*.csv)");
    if (path.isEmpty()) return;

    m_events = load_workload_csv(path.toStdString().c_str());
    m_loaded = true;

    // Buttons freischalten
    m_btnRun->setEnabled(true);
    m_btnStep->setEnabled(true);
    m_btnReset->setEnabled(true);

    // Engine direkt vorbereiten
    setupEngine();

    updateLog("CSV geladen: " + path +
              " | Events: " + QString::number((int)m_events.len));
    m_statusLabel->setText("CSV geladen. Bereit für Simulation.");
}

// ─── Alles auf einmal durchlaufen ─────────────────────────────────────────────
void MainWindow::onRunSimulation() {
    if (!m_loaded) return;

    setupEngine(); // Reset + neu konfigurieren
    engine_run(&m_engine, 100000);

    Metrics& met = m_engine.m;
    QString result = QString(
        "▶ Simulation abgeschlossen:\n"
        "  Deadlocks: %1 | Aborts: %2 | Rollbacks: %3\n"
        "  Grants:    %4 | Denies: %5 | Defers:    %6"
    ).arg(met.deadlocks_detected).arg(met.aborts).arg(met.rollbacks)
     .arg(met.grants).arg(met.denies).arg(met.defers);

    updateLog(result);
    updateTable();
    m_statusLabel->setText("Simulation fertig (Run All).");

    // Step deaktivieren, da Simulation fertig
    m_btnStep->setEnabled(false);
}

// ─── Einen Zeitschritt weiter ──────────────────────────────────────────────────
void MainWindow::onNextStep() {
    if (!m_loaded || !m_engineReady) return;

    // Merke Metrics VOR dem Schritt
    int grantsBefore  = m_engine.m.grants;
    int defersBefore  = m_engine.m.defers;
    int abortsBefore  = m_engine.m.aborts;
    int deadlocksBefore = m_engine.m.deadlocks_detected;

    // Nur einen Zeitschritt weiter
    m_currentTime++;
    engine_run(&m_engine, m_currentTime);

    // Was hat sich geändert?
    Metrics& met = m_engine.m;
    QString logMsg = QString("Zeit %1: ").arg(m_currentTime);

    if (met.grants > grantsBefore)
        logMsg += QString("GRANT (+%1)  ").arg(met.grants - grantsBefore);
    if (met.defers > defersBefore)
        logMsg += QString("DEFER (+%1)  ").arg(met.defers - defersBefore);
    if (met.aborts > abortsBefore)
        logMsg += QString("ABORT (+%1)  ").arg(met.aborts - abortsBefore);
    if (met.deadlocks_detected > deadlocksBefore)
        logMsg += QString("DEADLOCK erkannt! ");
    if (logMsg == QString("Zeit %1: ").arg(m_currentTime))
        logMsg += "— kein Event";

    updateLog(logMsg);
    updateTable();
    m_statusLabel->setText(QString("Schritt: Zeit %1 | Grants: %2 | Defers: %3 | Aborts: %4")
                           .arg(m_currentTime).arg(met.grants).arg(met.defers).arg(met.aborts));

    // Wenn keine Events mehr in der Queue → fertig
    if (m_engine.qlen == 0) {
        updateLog("Alle Events verarbeitet.");
        m_btnStep->setEnabled(false);
    }
}

// ─── Reset ────────────────────────────────────────────────────────────────────
void MainWindow::onResetSimulation() {
    setupEngine();
    m_allocTable->clearContents();
    m_allocTable->setRowCount(0);
    m_allocTable->setColumnCount(0);
    m_logView->clear();
    m_btnStep->setEnabled(true);
    updateLog("Simulation zurückgesetzt.");
    m_statusLabel->setText("Reset. Bereit für neuen Durchlauf.");
}

// ─── Tabelle aktualisieren ────────────────────────────────────────────────────
void MainWindow::updateTable() {
    SystemState& s = m_engine.s;
    m_allocTable->setRowCount(s.nprocs);
    m_allocTable->setColumnCount(s.nres);

    QStringList colHeaders, rowHeaders;
    for (int r = 0; r < s.nres; r++)
        colHeaders << "R" + QString::number(s.res[r].rid);
    for (int p = 0; p < s.nprocs; p++)
        rowHeaders << "P" + QString::number(s.procs[p].pid);

    m_allocTable->setHorizontalHeaderLabels(colHeaders);
    m_allocTable->setVerticalHeaderLabels(rowHeaders);

    for (int p = 0; p < s.nprocs; p++) {
        for (int r = 0; r < s.nres; r++) {
            int val = s.alloc[p][r];
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(val));
            item->setTextAlignment(Qt::AlignCenter);

            ProcState st = s.procs[p].state;
            if      (st == PS_BLOCKED)     item->setBackground(QColor(255, 80, 80));   // Rot
            else if (st == PS_ABORTED)     item->setBackground(QColor(180, 0, 0));     // Dunkelrot
            else if (st == PS_ROLLED_BACK) item->setBackground(QColor(255, 165, 0));   // Orange
            else if (st == PS_FINISHED)    item->setBackground(QColor(180, 180, 180)); // Grau
            else if (val > 0)              item->setBackground(QColor(100, 200, 100)); // Grün

            m_allocTable->setItem(p, r, item);
        }
    }
    m_allocTable->resizeColumnsToContents();
}

// ─── Log ──────────────────────────────────────────────────────────────────────
void MainWindow::updateLog(const QString& text) {
    m_logView->append(text);
}
