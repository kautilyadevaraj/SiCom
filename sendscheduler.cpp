#include "sendscheduler.h"

SendScheduler::SendScheduler(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this))
{
    // QTimer::timeout fires on every interval tick
    connect(m_timer, &QTimer::timeout, this, &SendScheduler::onTick);
}

void SendScheduler::setPayload(const QByteArray &data) { m_payload = data; }
void SendScheduler::setMode(Mode mode)                 { m_mode = mode; }
void SendScheduler::setIntervalMs(int ms)              { m_intervalMs = qMax(50, ms); }
void SendScheduler::setBurstCount(int count)           { m_burstCount = qMax(1, count); }
bool SendScheduler::isRunning() const                  { return m_timer->isActive(); }
SendScheduler::Mode SendScheduler::mode() const        { return m_mode; }

void SendScheduler::start() {
    if (m_payload.isEmpty()) return;
    m_sentCount = 0;

    if (m_mode == Mode::Once) {
        // No timer needed — fire once immediately and finish
        emit sendRequested(m_payload);
        emit progressUpdated(1, 1);
        emit finished();
        return;
    }
    // Interval and Burst both use the timer; send the first byte immediately
    // so the user doesn't wait one full interval before anything happens
    onTick();
    if (m_timer->isActive()) return; // onTick() may have stopped timer (Burst of 1)
    m_timer->start(m_intervalMs);
}

void SendScheduler::stop() {
    if (m_timer->isActive()) {
        m_timer->stop();
        emit finished();
    }
}

void SendScheduler::onTick() {
    emit sendRequested(m_payload);
    m_sentCount++;

    if (m_mode == Mode::Burst) {
        emit progressUpdated(m_sentCount, m_burstCount);
        if (m_sentCount >= m_burstCount) {
            m_timer->stop();
            emit finished();
        }
    } else {
        // Interval mode: no burst cap, progress is just "N sent so far"
        emit progressUpdated(m_sentCount, -1); // -1 = no total (open-ended)
    }
}