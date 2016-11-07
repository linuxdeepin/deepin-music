/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "inotifyengine.h"

#include <sys/inotify.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <QDebug>
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QSocketNotifier>

class InotifyEnginePrivate
{
public:
    QMap<int, QString>          m_paths;
    QMap<QString, QDir>         m_dirs;
    QMap<QString, int>          m_descriptors;
    int                         m_inotifyFd     = -1;
};


InotifyEngine::InotifyEngine(QObject *parent)
    : QObject(parent), d(new InotifyEnginePrivate)
{
    d->m_inotifyFd = -1;
#ifdef IN_CLOEXEC
    d->m_inotifyFd = inotify_init1(IN_CLOEXEC);
#endif
    if (d->m_inotifyFd == -1) {
        d->m_inotifyFd = inotify_init();
        if (d->m_inotifyFd == -1) {
            qDebug() << "Fail to initialize inotify";
            return;
        }

    }
    auto m_notifier = new QSocketNotifier(d->m_inotifyFd, QSocketNotifier::Read, this);

    fcntl(d->m_inotifyFd, F_SETFD, FD_CLOEXEC);
    connect(m_notifier, &QSocketNotifier::activated, this, &InotifyEngine::readFromInotify);
}

InotifyEngine::~InotifyEngine()
{

}

void InotifyEngine::watchAllDirectory(const QString &path)
{
    QDir dir(path);

    if (!dir.exists()) {
        return;
    }

    if (watchDirectory(dir)) {
        return;
    }

    if (!dir.isRoot()) {
        dir.cdUp();
        watchAllDirectory(dir.absolutePath());
    }
}

void InotifyEngine::addPaths(const QStringList &paths)
{
    for (auto &path : paths) {
        QFileInfo fi(path);
        if (fi.isDir()) {
            watchAllDirectory(fi.absoluteFilePath());
        } else {
            watchAllDirectory(fi.absolutePath());
        }
    }
}

void InotifyEngine::addPath(const QString &path)
{
    addPaths(QStringList() << path);
}

void InotifyEngine::readFromInotify(int /*socket*/)
{
    size_t buffSize = 0;
    ioctl(d->m_inotifyFd, FIONREAD, reinterpret_cast<char *>(&buffSize));

    QVarLengthArray<char, 4096> buffer(static_cast<int>(buffSize));
    auto readSize = read(d->m_inotifyFd, buffer.data(), buffSize);
    char *at = buffer.data();
    char *const end = at + readSize;

    QHash<int, inotify_event *> eventForId;
    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);
        if (eventForId.contains(event->wd)) {
            eventForId[event->wd]->mask |= event->mask;
        } else {
            eventForId.insert(event->wd, event);
        }

        handleInotifyEvent(event);

        at += sizeof(inotify_event) + event->len;
    }

    QHash<int, inotify_event *>::const_iterator it = eventForId.constBegin();
    while (it != eventForId.constEnd()) {
        const inotify_event &event = **it;
        ++it;
        if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
            inotify_rm_watch(d->m_inotifyFd, event.wd);
        }
    }
}

void InotifyEngine::handleInotifyEvent(inotify_event *event)
{
    int id = event->wd;

    QString filepath = d->m_paths.value(id);
    if (filepath.isEmpty()) {
        return;
    }


    if (d->m_dirs.contains(filepath)) {
        auto dir =  d->m_dirs.value(filepath);
        filepath = dir.absoluteFilePath(event->name);
    } else {
        return;
    }

    if (event->mask & IN_MOVED_FROM
            || event->mask & IN_DELETE) {
//        qDebug() << "fileRemoved" << filepath;
        emit fileRemoved(filepath);
    }
}

bool InotifyEngine::watchDirectory(QDir &dir)
{
    uint32_t dirMask  = 0 | IN_ATTRIB | IN_MOVE | IN_CREATE | IN_DELETE
                        | IN_DELETE_SELF | IN_MOVE_SELF;
    uint32_t fileMask = 0 | IN_ATTRIB | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE
                        | IN_MODIFY | IN_MOVE | IN_MOVE_SELF | IN_DELETE_SELF;

    auto  isDir = true;
    auto  filepath = QString::fromLocal8Bit(QFile::encodeName(dir.absolutePath()));
    if (d->m_descriptors.contains(filepath)) {
//        qDebug() << "skip filepath" << filepath;
        return true;
    }

    auto mask = isDir ? dirMask : fileMask;
    int wd = inotify_add_watch(d->m_inotifyFd,
                               QFile::encodeName(filepath),
                               mask);

    if (wd < 0) {
        qDebug() << "inotify_add_watch failed" << wd << QFile::encodeName(filepath);
        return true;
    }

//    qDebug() << "inotify_add_watch " << wd << dir.absolutePath();

    d->m_descriptors.insert(filepath, wd);
    d->m_paths.insert(wd, filepath);
    if (isDir) {
        d->m_dirs.insert(filepath, QDir(filepath));
    }
    return false;
}
