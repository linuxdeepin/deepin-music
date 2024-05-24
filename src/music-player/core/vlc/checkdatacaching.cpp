// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkdatacaching.h"

#include "player.h"
#include <QDebug>

#define MAXBLOCKSIZE 50

extern int g_dataCacheBlock;
extern bool  g_stillPlay;

#define POSITION_STEP_COUNT 6

CheckDataCachingThread::CheckDataCachingThread(QObject *parent)
    : QThread(parent)
{

}

void CheckDataCachingThread::setThreadPause(bool pause)
{
    m_pause = pause;
    if (g_stillPlay)
        g_stillPlay = !m_pause;
}

void CheckDataCachingThread::run()
{
    while (!m_bExit) {
        if (m_pause) {
            msleep(500);
            continue;
        }
        if (g_dataCacheBlock > MAXBLOCKSIZE) {
                if (m_pause) continue;
                g_stillPlay = true;
                emit sigPusedDecode();
        } else {
                if (m_pause) continue;
                g_stillPlay = false;
                emit sigResumeDecode();
        }
        msleep(500);
    }
}
