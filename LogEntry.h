#pragma once
#include <QDateTime>
#include <QByteArray>
#include <QString>

// Plain data struct. Stored in MainWindow::m_logEntries.
// Used by FileManager::saveSnapshot() to write the full log to disk.
struct LogEntry {
    QDateTime  timestamp;
    QString    direction; // "TX" or "RX"
    QByteArray rawData;   // raw bytes — FileManager decides how to format them
};