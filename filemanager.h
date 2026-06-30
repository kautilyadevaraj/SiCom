#pragma once
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QList>
#include "LogEntry.h"
#include "dataconvertor.h"

class FileManager : public QObject {
    Q_OBJECT
public:
    enum class LogFormat { PlainText, CSV };
    enum class FileMode  { Text, Binary };

    static const int CHUNK_SIZE = 256; // bytes per chunk in Binary mode

    explicit FileManager(QObject *parent = nullptr);
    ~FileManager();

    // ── Auto-logging ──────────────────────────────────────────────────────
    bool startLogging(const QString &filePath, LogFormat format, bool append = false);
    void stopLogging();
    bool isLogging() const;
    // Call from MainWindow::appendToLog() on every TX/RX entry
    void logEntry(const QString &direction, const QByteArray &data, const QDateTime &ts, DataConverter::Format fmt);

    // ── Snapshot save ─────────────────────────────────────────────────────
    // Exports the full m_logEntries list to a file in one shot
    bool saveSnapshot(const QString &filePath, LogFormat format,
                      const QList<LogEntry> &entries, DataConverter::Format fmt);

    // ── File loading for send ─────────────────────────────────────────────
    bool loadFile(const QString &filePath, FileMode mode);
    bool       hasMoreLines()  const;
    QByteArray nextLine();           // advances internal pointer each call
    QByteArray allBytes()      const;
    int        totalLines()    const;
    int        currentLine()   const;
    void       resetPosition();
    QString    loadedFilePath() const;
    qint64     loadedFileSize() const;

    QString   currentLogPath()   const;
    LogFormat currentLogFormat() const;

signals:
    void loggingStarted(const QString &filePath);
    void loggingStopped();
    void loggingError(const QString &message);

private:
    QString formatPlainText(const QString &dir, const QByteArray &data, const QDateTime &ts, DataConverter::Format fmt);
    QString formatCSV      (const QString &dir, const QByteArray &data, const QDateTime &ts, DataConverter::Format fmt);

    // Auto-log state
    QFile       *m_logFile   = nullptr;
    QTextStream *m_logStream = nullptr;
    LogFormat    m_logFormat = LogFormat::PlainText;
    bool         m_logging   = false;

    // File-send state
    QList<QByteArray> m_lines;
    int               m_lineIndex = 0;
    QString           m_filePath;
    qint64            m_fileSize  = 0;

    QString   m_currentLogPath;
    LogFormat m_currentLogFormat = LogFormat::PlainText;
};