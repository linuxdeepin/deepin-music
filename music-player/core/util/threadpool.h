#pragma once

#include <QThread>

#include <util/singleton.h>

class ThreadPool : public QObject, public DMusic::DSingleton<ThreadPool>
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = 0);
    ~ThreadPool();

    QThread *newThread();
    void moveToNewThread(QObject *obj);
    void manager(QThread *thread);
    void quitAll();

private:
    friend class DMusic::DSingleton<ThreadPool>;

    QList<QThread *> m_pool;
};

