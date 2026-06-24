#include "SerialManager.h"
#include <QtSerialPort/QSerialPortInfo>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent), m_port(new QSerialPort(this))
{
    // readyRead fires whenever the OS hands us new bytes — no polling thread needed.
    connect(m_port, &QSerialPort::readyRead,      this, &SerialManager::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred,  this, &SerialManager::onPortError);
}

void SerialManager::configure(const SerialConfig &cfg) {
    m_port->setPortName(cfg.portName);
    m_port->setBaudRate(cfg.baudRate);
    m_port->setDataBits(cfg.dataBits);
    m_port->setParity(cfg.parity);
    m_port->setStopBits(cfg.stopBits);
    m_port->setFlowControl(cfg.flowControl);
}

bool SerialManager::open() {
    if (!m_port->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_port->errorString());
        return false;
    }
    emit connectionChanged(true);
    return true;
}

void SerialManager::close() {
    if (m_port->isOpen()) m_port->close();
    emit connectionChanged(false);
}

bool SerialManager::isOpen() const { return m_port->isOpen(); }

bool SerialManager::sendBytes(const QByteArray &data) {
    if (!m_port->isOpen() || data.isEmpty()) return false;
    qint64 written = m_port->write(data);
    if (written == data.size()) {
        emit bytesSent(data);   // lets MainWindow log the TX entry
        return true;
    }
    emit errorOccurred("Partial write: " + QString::number(written) +
                       "/" + QString::number(data.size()) + " bytes sent.");
    return false;
}

QStringList SerialManager::availablePorts() {
    QStringList list;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        list << info.portName();
    return list;
}

void SerialManager::onReadyRead() {
    const QByteArray data = m_port->readAll();
    if (!data.isEmpty()) emit bytesReceived(data);
}

void SerialManager::onPortError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) return;
    emit errorOccurred(m_port->errorString());
    // ResourceError = device physically unplugged. Close cleanly so UI updates.
    if (error == QSerialPort::ResourceError) close();
}