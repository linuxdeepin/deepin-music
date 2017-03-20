#include "threadpool.h"

#include <QDebug>

ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{

}

ThreadPool::~ThreadPool()
{
    qDebug() << "destory ThreadPool";
    quitAll();
    qDebug() << "ThreadPool destoryed";

}

QThread *ThreadPool::newThread()
{
    auto thread = new QThread;
//    qDebug() << "add <<<<<<<" << thread;
    m_pool.push_back(thread);
    return thread;
}

void ThreadPool::moveToNewThread(QObject *obj)
{
    auto work = newThread();
    obj->moveToThread(work);
    work->start();
}

void ThreadPool::manager(QThread *thread)
{
//    qDebug() << "manager <<<<<<<" << thread;
    m_pool.push_back(thread);
}

void ThreadPool::quitAll()
{
    for (auto thread : m_pool) {
//        qDebug() << thread;
        thread->quit();
        thread->wait();
    }
    qDebug() << "all thread quit";
}
