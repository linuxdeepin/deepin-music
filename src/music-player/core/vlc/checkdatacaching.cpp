// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkdatacaching.h"

#include "player.h"
#include <QDebug>

#define MAXBLOCKSIZE 50
#define MINBLOCKSIZE 30

extern int g_dataCacheBlock;

#define POSITION_STEP_COUNT 6

CheckDataCachingThread::CheckDataCachingThread(QObject *parent)
    : QThread(parent)
{

}

void CheckDataCachingThread::setThreadPause(bool pause)
{
    m_pause = pause;
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
                emit sigPusedDecode();
        } else if (g_dataCacheBlock < MINBLOCKSIZE) {
                if (m_pause) continue;
                emit sigResumeDecode();
        }
        msleep(500);
    }
}
