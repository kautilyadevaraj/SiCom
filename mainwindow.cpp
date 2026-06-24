#include "mainwindow.h"
#include "ui_mainwindow.h"          // auto-generated from mainwindow.ui by AUTOUIC
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>

// ── Constructor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serial(new SerialManager(this))
{
    ui->setupUi(this);
    setWindowTitle("Serial Terminal");
    setupCombos();

    // ── UI → slots ───────────────────────────────────────────────────────
    connect(ui->refreshButton,      &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);
    connect(ui->connectButton,      &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(ui->clearLogButton,     &QPushButton::clicked,
            this, &MainWindow::onClearLogClicked);
    connect(ui->saveLogButton,      &QPushButton::clicked,
            this, &MainWindow::onSaveLogClicked);
    connect(ui->sendButton,         &QPushButton::clicked,
            this, &MainWindow::onSendClicked);
    connect(ui->sendLineEdit,       &QLineEdit::returnPressed,
            this, &MainWindow::onSendClicked);

    // ── Validation and format switching ─────────────────────────────
    connect(ui->sendLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSendInputChanged);
    connect(ui->sendFormatComboBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::onSendFormatChanged);
    connect(ui->displayFormatComboBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::onDisplayFormatChanged);

    // ── SerialManager → slots ─────────────────────────────────────────────
    connect(m_serial, &SerialManager::bytesReceived,   this, &MainWindow::onBytesReceived);
    connect(m_serial, &SerialManager::bytesSent,       this, &MainWindow::onBytesSent);
    connect(m_serial, &SerialManager::connectionChanged,this, &MainWindow::onConnectionChanged);
    connect(m_serial, &SerialManager::errorOccurred,   this, &MainWindow::onSerialError);

    //SendManager Slots
    // Schedule mode UI
    connect(ui->scheduleModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onScheduleModeChanged);
    // Qt5: same — QOverload was already needed for QComboBox

    connect(ui->stopSendButton, &QPushButton::clicked,
            this, &MainWindow::onStopSendClicked);

    // Scheduler → SerialManager (the core wiring)
    // Scheduler never calls SerialManager directly — it signals through MainWindow
    connect(m_scheduler, &SendScheduler::sendRequested,
            m_serial, &SerialManager::sendBytes);

    // Scheduler feedback → UI
    connect(m_scheduler, &SendScheduler::finished,
            this, &MainWindow::onSchedulerFinished);
    connect(m_scheduler, &SendScheduler::progressUpdated,
            this, &MainWindow::onSchedulerProgress);

    // ── Initial state ─────────────────────────────────────────────────────
    onRefreshClicked();                 // populate port list on startup
    onConnectionChanged(false);         // set all widgets to disconnected state
    ui->validationLabel->hide();        // hidden until user types invalid input
    // Hide various send control UI elements
    ui->intervalLabel->setVisible(false);
    ui->intervalSpinBox->setVisible(false);
    ui->burstLabel->setVisible(false);
    ui->burstCountSpinBox->setVisible(false);
}

MainWindow::~MainWindow() { delete ui; }

// ── Setup ─────────────────────────────────────────────────────────────────────

void MainWindow::setupCombos() {
    ui->baudComboBox->addItems({"1200","2400","4800","9600","19200",
                                "38400","57600","115200","230400","921600"});
    ui->baudComboBox->setCurrentText("115200");

    ui->dataBitsComboBox->addItems({"5","6","7","8"});
    ui->dataBitsComboBox->setCurrentText("8");

    ui->parityComboBox->addItems({"None","Even","Odd","Space","Mark"});
    ui->stopBitsComboBox->addItems({"1","1.5","2"});
    ui->flowControlComboBox->addItems({"None","RTS/CTS","XON/XOFF"});

    // Format combos — order must match DataConverter::formatFromIndex()
    const QStringList fmts = {"ASCII","Hex","Binary","Decimal"};
    ui->sendFormatComboBox->addItems(fmts);
    ui->displayFormatComboBox->addItems(fmts);

    m_scheduler = new SendScheduler(this);
    // Send Scheduler combo setup
    ui->scheduleModeComboBox->addItems({"Once" , "Interval" , "Burst"});
    ui->scheduleModeComboBox->setCurrentIndex(0);
}

// ── Port panel slots ──────────────────────────────────────────────────────────

void MainWindow::onRefreshClicked() {
    const QString current = ui->portComboBox->currentText(); // preserve selection
    ui->portComboBox->clear();
    ui->portComboBox->addItems(SerialManager::availablePorts());
    // Restore previous selection if it still exists after refresh
    int idx = ui->portComboBox->findText(current);
    if (idx >= 0) ui->portComboBox->setCurrentIndex(idx);
}

void MainWindow::onConnectClicked() {
    if (m_serial->isOpen()) { m_serial->close(); return; }

    if (ui->portComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "No Port", "Select a COM port first.");
        return;
    }

    // Map each combo index to the corresponding QSerialPort enum value.
    // Using static arrays avoids a chain of if/else for each setting.
    static const QSerialPort::DataBits dbMap[] = {
        QSerialPort::Data5, QSerialPort::Data6,
        QSerialPort::Data7, QSerialPort::Data8
    };
    static const QSerialPort::Parity parMap[] = {
        QSerialPort::NoParity,    QSerialPort::EvenParity,
        QSerialPort::OddParity,   QSerialPort::SpaceParity,
        QSerialPort::MarkParity
    };
    static const QSerialPort::StopBits sbMap[] = {
        QSerialPort::OneStop, QSerialPort::OneAndHalfStop, QSerialPort::TwoStop
    };
    static const QSerialPort::FlowControl fcMap[] = {
        QSerialPort::NoFlowControl,
        QSerialPort::HardwareControl,
        QSerialPort::SoftwareControl
    };

    SerialConfig cfg;
    cfg.portName    = ui->portComboBox->currentText();
    cfg.baudRate    = ui->baudComboBox->currentText().toInt();
    cfg.dataBits    = dbMap[ui->dataBitsComboBox->currentIndex()];
    cfg.parity      = parMap[ui->parityComboBox->currentIndex()];
    cfg.stopBits    = sbMap[ui->stopBitsComboBox->currentIndex()];
    cfg.flowControl = fcMap[ui->flowControlComboBox->currentIndex()];

    m_serial->configure(cfg);
    m_serial->open(); // result arrives via onConnectionChanged / onSerialError
}

// ── Log panel slots ───────────────────────────────────────────────────────────

void MainWindow::onClearLogClicked() { ui->logTextEdit->clear(); }

void MainWindow::onSaveLogClicked() {
    // TODO: Phase 4 — FileManager::startLogging / stopLogging
    statusBar()->showMessage("Save log: coming in Phase 4.", 3000);
}

// ── Send panel slots ──────────────────────────────────────────────────────────

void MainWindow::onSendClicked() {
    if (!m_serial->isOpen()) {
        statusBar()->showMessage("Not connected.", 2000);
        return;
    }
    if (m_scheduler->isRunning()) {
        statusBar()->showMessage("Already sending. Press Stop first.", 2000);
        return;
    }

    const QString input = ui->sendLineEdit->text().trimmed();
    if (input.isEmpty()) return;

    const DataConverter::Format fmt = currentSendFormat();
    if (!DataConverter::isValid(input, fmt)) {
        statusBar()->showMessage("Invalid input for selected format.", 3000);
        return;
    }

    QByteArray data = DataConverter::parse(input, fmt);
    if (fmt == DataConverter::Format::Ascii && ui->appendNewlineCheckBox->isChecked())
        data.append('\n');

    // Configure scheduler from UI controls
    static const SendScheduler::Mode modeMap[] = {
        SendScheduler::Mode::Once,
        SendScheduler::Mode::Interval,
        SendScheduler::Mode::Burst
    };
    m_scheduler->setPayload(data);
    m_scheduler->setMode(modeMap[ui->scheduleModeComboBox->currentIndex()]);
    m_scheduler->setIntervalMs(ui->intervalSpinBox->value());
    m_scheduler->setBurstCount(ui->burstCountSpinBox->value());
    m_scheduler->start();

    // Disable send controls while scheduler is active (Once re-enables immediately
    // via onSchedulerFinished, so there's no visible flicker for Once mode)
    ui->sendButton->setEnabled(false);
    ui->stopSendButton->setEnabled(true);
    ui->sendLineEdit->setEnabled(false);
}

// ── Validation slots ────────────────────────────────────────────────────

void MainWindow::onSendInputChanged(const QString &text) {
    const DataConverter::Format fmt = currentSendFormat();
    // ASCII is always valid; don't show errors on an empty field
    const bool valid = text.isEmpty() || DataConverter::isValid(text, fmt);
    ui->validationLabel->setVisible(!valid);
    // Red border signals the problem directly on the input field
    ui->sendLineEdit->setStyleSheet(valid ? "" : "QLineEdit { border: 1px solid red; }");
}

void MainWindow::onSendFormatChanged(int /*index*/) {
    // Update placeholder so the user knows what format to type in
    static const char *hints[] = {
        "e.g.  Hello World",
        "e.g.  AA BB CC FF",
        "e.g.  10101010 11001100",
        "e.g.  170 204 255"
    };
    ui->sendLineEdit->setPlaceholderText(
        hints[ui->sendFormatComboBox->currentIndex()]
        );
    // Re-run validation against the new format without waiting for more typing
    onSendInputChanged(ui->sendLineEdit->text());
}

void MainWindow::onDisplayFormatChanged(int /*index*/) {
    // New entries will use the new format automatically via appendToLog().
    // Optionally note the change in the log so the user can see where format switched.
    ui->logTextEdit->append(
        "<span style='color:gray'>── display format: "
        + DataConverter::formatName(currentDisplayFormat()) + " ──</span>"
        );
}

// ── SerialManager slots ───────────────────────────────────────────────────────

void MainWindow::onBytesReceived(const QByteArray &data) { appendToLog("RX", data); }
void MainWindow::onBytesSent   (const QByteArray &data) { appendToLog("TX", data); }

void MainWindow::onConnectionChanged(bool isOpen) {
    ui->connectButton->setText(isOpen ? "Disconnect" : "Connect");
    // Serial settings cannot be changed while port is open
    const bool canConfigure = !isOpen;
    ui->portComboBox->setEnabled(canConfigure);
    ui->baudComboBox->setEnabled(canConfigure);
    ui->dataBitsComboBox->setEnabled(canConfigure);
    ui->parityComboBox->setEnabled(canConfigure);
    ui->stopBitsComboBox->setEnabled(canConfigure);
    ui->flowControlComboBox->setEnabled(canConfigure);
    ui->refreshButton->setEnabled(canConfigure);
    // Send controls only usable when connected
    ui->sendButton->setEnabled(isOpen);
    ui->sendLineEdit->setEnabled(isOpen);
    statusBar()->showMessage(
        isOpen ? "Connected: " + ui->portComboBox->currentText() : "Disconnected"
        );

    ui->sendButton->setEnabled(isOpen);
    ui->sendLineEdit->setEnabled(isOpen);
    ui->stopSendButton->setEnabled(false);   // always off on connect/disconnect
    if (!isOpen && m_scheduler->isRunning()) // device unplugged mid-send
        m_scheduler->stop();
}

void MainWindow::onSerialError(const QString &message) {
    statusBar()->showMessage("Error: " + message, 5000);
}

//SendScheduler Slots
void MainWindow::onStopSendClicked() {
    m_scheduler->stop(); // triggers onSchedulerFinished via finished() signal
}

void MainWindow::onSchedulerFinished() {
    // Re-enable send controls and clear progress message
    ui->sendButton->setEnabled(m_serial->isOpen());
    ui->stopSendButton->setEnabled(false);
    ui->sendLineEdit->setEnabled(m_serial->isOpen());
    statusBar()->showMessage("Done.", 2000);
}

void MainWindow::onSchedulerProgress(int sent, int total) {
    if (total == -1)
        // Interval mode — open-ended, just show count
        statusBar()->showMessage(QString("Sending... %1 sent").arg(sent));
    else
        statusBar()->showMessage(
            QString("Sending %1 / %2").arg(sent).arg(total)
            );
}

void MainWindow::onScheduleModeChanged(int index) {
    // Show/hide interval and burst controls based on selected mode
    const bool showInterval = (index == 1 || index == 2); // Interval or Burst
    const bool showBurst    = (index == 2);                // Burst only

    ui->intervalLabel->setVisible(showInterval);
    ui->intervalSpinBox->setVisible(showInterval);
    ui->burstLabel->setVisible(showBurst);
    ui->burstCountSpinBox->setVisible(showBurst);
}

// ── Private helpers ───────────────────────────────────────────────────────────

void MainWindow::appendToLog(const QString &direction, const QByteArray &data) {
    const QString ts    = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    const QString color = (direction == "TX") ? "#0055ff" : "#00aa44";
    // [P2] Use DataConverter to display in whatever format the user selected.
    // In Phase 1 (before P2 combo exists), replace currentDisplayFormat()
    // with DataConverter::Format::Hex as a temporary default.
    const QString shown = DataConverter::display(data, currentDisplayFormat());
    ui->logTextEdit->append(
        QString("<span style='color:gray'>[%1]</span> "
                "<span style='color:%2;font-weight:bold'>%3</span>&nbsp;%4")
            .arg(ts, color, direction, shown.toHtmlEscaped())
        );
}

// [P2] Read current format selection from the combo boxes
DataConverter::Format MainWindow::currentSendFormat() const {
    return DataConverter::formatFromIndex(ui->sendFormatComboBox->currentIndex());
}
DataConverter::Format MainWindow::currentDisplayFormat() const {
    return DataConverter::formatFromIndex(ui->displayFormatComboBox->currentIndex());
}