#pragma once
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include "SerialConfig.h"

// Owns and operates one QSerialPort. The rest of the app never touches
// QSerialPort directly — it talks only to these signals and slots.
class SerialManager : public QObject {
    Q_OBJECT
public:
    explicit SerialManager(QObject *parent = nullptr);

    void configure(const SerialConfig &config); // apply settings (port closed)
    bool open();                                 // returns false on failure
    void close();
    bool isOpen() const;
    bool sendBytes(const QByteArray &data);      // returns false if port closed

    static QStringList availablePorts();         // for populating portComboBox

signals:
    void bytesReceived(const QByteArray &data);  // fired by readyRead handler
    void bytesSent(const QByteArray &data);      // echo for the TX log entry
    void connectionChanged(bool isOpen);          // drive UI enable/disable
    void errorOccurred(const QString &message);  // display in status bar

private slots:
    void onReadyRead();
    void onPortError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_port;
};