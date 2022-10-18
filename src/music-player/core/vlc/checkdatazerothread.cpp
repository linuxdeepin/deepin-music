#include "checkdatazerothread.h"

#include "player.h"
#include <QDebug>

extern int g_playbackStatus;

#define POSITION_STEP_COUNT 6

CheckDataZeroThread::CheckDataZeroThread(QObject *parent)
    : QThread(parent)
{

}

void CheckDataZeroThread::initTimeParams()
{
    if (Player::getInstance()->getActiveMeta().localPath.isEmpty())
        return;
    m_currentTime = Player::getInstance()->position();
    m_duration = Player::getInstance()->getActiveMeta().length;
    m_step = (m_duration - m_currentTime) / POSITION_STEP_COUNT;
}

void CheckDataZeroThread::run()
{
    while (!m_bExit) {
        if (g_playbackStatus == 2) {
            // 发送最后一秒的数据，保证进度能显示到最后一秒
            if (m_currentTime > 0)
                emit sigExtraTime(m_duration);
            msleep(100);
            emit sigPlayNextSong();
            g_playbackStatus = 0;
            resetParam();
        }

        if (g_playbackStatus == 1 && m_currentTime > 0) {
            if (Player::getInstance()->getActiveMeta().localPath.isEmpty()) {
                resetParam();
                continue;
            }

            if (m_currentTime <= m_duration) {
                m_currentTime += m_step;
                emit sigExtraTime(m_currentTime > m_duration ? m_duration : m_currentTime);
            }
        }
        msleep(300);
    }
}

void CheckDataZeroThread::resetParam()
{
    m_currentTime = 0;
    m_step = 0;
    m_duration = 0;
}
