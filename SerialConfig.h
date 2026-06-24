#pragma once
#include <QString>
#include <QtSerialPort/QSerialPort>

// Plain data struct — carries all port settings between UI and SerialManager.
// No methods, no Qt parent. Lives in models/ to avoid circular includes.
struct SerialConfig {
    QString portName;
    qint32  baudRate    = 115200;
    QSerialPort::DataBits    dataBits    = QSerialPort::Data8;
    QSerialPort::Parity      parity      = QSerialPort::NoParity;
    QSerialPort::StopBits    stopBits    = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};