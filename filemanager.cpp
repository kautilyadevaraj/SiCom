#include "filemanager.h"
#include <QFileInfo>
#include <QTextStream>

FileManager::FileManager(QObject *parent) : QObject(parent) {}
FileManager::~FileManager() { stopLogging(); }

// ── Auto-logging ──────────────────────────────────────────────────────────────

bool FileManager::startLogging(const QString &filePath, LogFormat format, bool append) {
    stopLogging(); // close any previous session cleanly
    m_currentLogPath   = filePath;   // store so MainWindow can restart
    m_currentLogFormat = format;
    m_logFile = new QFile(filePath, this);
    QIODevice::OpenMode openMode = QIODevice::WriteOnly | QIODevice::Text;
    if (append) openMode |= QIODevice::Append;

    if (!m_logFile->open(openMode)) {
        emit loggingError("Cannot open: " + m_logFile->errorString());
        delete m_logFile; m_logFile = nullptr;
        return false;
    }
    m_logStream = new QTextStream(m_logFile);
    m_logFormat = format;

    if (format == LogFormat::CSV && !append)
        *m_logStream << "Timestamp,Direction,Data\n"; // CSV header for new files

    m_logging = true;
    emit loggingStarted(filePath);
    return true;
}

void FileManager::stopLogging() {
    if (!m_logging) return;
    m_logStream->flush();
    delete m_logStream; m_logStream = nullptr;
    m_logFile->close();
    delete m_logFile;   m_logFile = nullptr;
    m_logging = false;
    emit loggingStopped();
}

bool FileManager::isLogging() const { return m_logging; }

void FileManager::logEntry(const QString &direction, const QByteArray &data,
                           const QDateTime &ts, DataConverter::Format fmt) {
    if (!m_logging || !m_logStream) return;
    QString line = (m_logFormat == LogFormat::CSV)
                       ? formatCSV(direction, data, ts, fmt)
                       : formatPlainText(direction, data, ts, fmt);
    *m_logStream << line << "\n";
    m_logStream->flush(); // flush every write — prevents data loss on crash/unplug
}

// ── Snapshot save ─────────────────────────────────────────────────────────────

bool FileManager::saveSnapshot(const QString &filePath, LogFormat format,
                               const QList<LogEntry> &entries, DataConverter::Format fmt) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
    if (format == LogFormat::CSV) stream << "Timestamp,Direction,Data\n";
    for (const LogEntry &e : entries) {
        stream << ((format == LogFormat::CSV)
                   ? formatCSV(e.direction, e.rawData, e.timestamp, fmt)
                   : formatPlainText(e.direction, e.rawData, e.timestamp, fmt))
               << "\n";
    }
    return true;
}

// ── Private formatters ────────────────────────────────────────────────────────

QString FileManager::formatPlainText(const QString &dir, const QByteArray &data,
                                     const QDateTime &ts, DataConverter::Format fmt) {
    return QString("[%1] %2: %3")
    .arg(ts.toString("yyyy-MM-dd HH:mm:ss.zzz"),
         dir,
         DataConverter::display(data, fmt));
}

QString FileManager::formatCSV(const QString &dir, const QByteArray &data,
                               const QDateTime &ts, DataConverter::Format fmt) {
    return QString("%1,%2,\"%3\"")
    .arg(ts.toString("yyyy-MM-dd HH:mm:ss.zzz"),
         dir,
         DataConverter::display(data, fmt));
}

// ── File loading ──────────────────────────────────────────────────────────────

bool FileManager::loadFile(const QString &filePath, FileMode mode) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    m_lines.clear();
    m_lineIndex = 0;
    m_filePath  = filePath;
    m_fileSize  = QFileInfo(filePath).size();

    if (mode == FileMode::Text) {
        QTextStream stream(&file);
        while (!stream.atEnd())
            m_lines << (stream.readLine() + "\n").toLatin1();
    } else {
        // Binary: split into fixed chunks so large files don't overwhelm the device
        QByteArray all = file.readAll();
        for (int i = 0; i < all.size(); i += CHUNK_SIZE)
            m_lines << all.mid(i, CHUNK_SIZE);
    }
    return !m_lines.isEmpty();
}

bool       FileManager::hasMoreLines()  const { return m_lineIndex < m_lines.size(); }
QByteArray FileManager::nextLine()            { return m_lines.value(m_lineIndex++); }
int        FileManager::totalLines()    const { return m_lines.size(); }
int        FileManager::currentLine()   const { return m_lineIndex; }
void       FileManager::resetPosition()       { m_lineIndex = 0; }
QString    FileManager::loadedFilePath() const { return m_filePath; }
qint64     FileManager::loadedFileSize() const { return m_fileSize; }

QByteArray FileManager::allBytes() const {
    QByteArray all;
    for (const QByteArray &chunk : m_lines) all += chunk;
    return all;
}

QString FileManager::currentLogPath()   const { return m_currentLogPath; }
FileManager::LogFormat FileManager::currentLogFormat() const { return m_currentLogFormat; }