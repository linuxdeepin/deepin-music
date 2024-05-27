// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKDATACACHINGTHREAD_H
#define CHECKDATACACHINGTHREAD_H

#include <QThread>

class CheckDataCachingThread : public QThread
{
    Q_OBJECT
public:
    explicit CheckDataCachingThread(QObject *parent = nullptr);

    void quitThread() {m_bExit = true;}

    void setThreadPause(bool pause);

    bool getPause() {return m_pause;}

signals:
    void sigPusedDecode();
    void sigResumeDecode();

protected:
    void run() override;

private:
    bool m_pause = false;
    bool m_bExit = false;
};

#endif // CHECKDATAZEROTHREAD_H
