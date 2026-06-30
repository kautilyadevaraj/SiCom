#pragma once
#include <QMainWindow>
#include "SerialManager.h"
#include "dataconvertor.h"
#include "sendscheduler.h"
#include "filemanager.h"
#include "LogEntry.h"
#include <QTimer>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // ── Port panel ───────────────────────────────
    void onRefreshClicked();
    void onConnectClicked();

    // ── Log panel ────────────────────────────────
    void onClearLogClicked();
    void onSaveLogClicked();

    // ── Send panel ───────────────────────────────
    void onSendClicked();
    void onSendInputChanged(const QString &text);
    void onSendFormatChanged(int index);
    void onDisplayFormatChanged(int index);

    // ── SerialManager signals ────────────────────
    void onBytesReceived(const QByteArray &data);
    void onBytesSent(const QByteArray &data);
    void onConnectionChanged(bool isOpen);
    void onSerialError(const QString &message);

    //SendScheduler slots
    void onStopSendClicked();
    void onScheduleModeChanged(int index);
    void onSchedulerFinished();
    void onSchedulerProgress(int sent, int total);

    //FileManager Slots
    void onAutoLogToggled();
    void onBrowseFileClicked();
    void onSendFileClicked();
    void onStopFileSendClicked();
    void onFileSendModeChanged(int index);   // All at once vs Line by line
    void onFileModeChanged(int index);       // Text vs Binary
    void onFileSendTick();                   // timer tick — sends next line/chunk
    void onLoggingStarted(const QString &path);
    void onLoggingStopped();

private:
    void setupCombos();
    void appendToLog(const QString &direction, const QByteArray &data);
    DataConverter::Format currentSendFormat()    const;
    DataConverter::Format currentDisplayFormat() const;

    //FileManager Private Methods
    void updateFilePreview();   // reads first bytes of selected file into filePreviewLabel
    void onFileSendComplete();

    Ui::MainWindow  *ui;
    SerialManager   *m_serial;
    SendScheduler *m_scheduler;

    //FileManager Private members
    FileManager     *m_fileManager;
    QTimer          *m_fileSendTimer;
    QList<LogEntry>  m_logEntries;   // full log history for snapshot save
};