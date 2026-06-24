#pragma once
#include <QObject>
#include <QTimer>
#include <QByteArray>

// Decides WHEN to send. Never touches QSerialPort directly.
// Emits sendRequested() → MainWindow connects this to SerialManager::sendBytes().
class SendScheduler : public QObject {
    Q_OBJECT
public:
    enum class Mode { Once, Interval, Burst };

    explicit SendScheduler(QObject *parent = nullptr);

    // Call these before start()
    void setPayload(const QByteArray &data);
    void setMode(Mode mode);
    void setIntervalMs(int ms);      // used by Interval and Burst
    void setBurstCount(int count);   // used by Burst only

    void start();   // begin scheduled sending
    void stop();    // cancel — safe to call even if not running

    bool isRunning() const;
    Mode mode()     const;

signals:
    void sendRequested(const QByteArray &data); // connect to SerialManager::sendBytes
    void progressUpdated(int sent, int total);  // Burst: drive status bar ("3/5")
    void finished();                            // Once: immediately; Burst: after last send;
    // Interval: only after stop() is called

private slots:
    void onTick();  // called by QTimer on every interval

private:
    QTimer     *m_timer;
    QByteArray  m_payload;
    Mode        m_mode       = Mode::Once;
    int         m_intervalMs = 1000;
    int         m_burstCount = 5;
    int         m_sentCount  = 0;
};