#pragma once

#include <QObject>
#include <QScopedPointer>

#include <mediameta.h>

#include "util/singleton.h"

class MediaLibraryPrivate;
class MediaLibrary : public QObject, public DMusic::DSingleton<MediaLibrary>
{
    Q_OBJECT
public:
    ~MediaLibrary();

    MetaPtr meta(const QString &hash);
    bool contains(const QString &hash) const;
    bool isEmpty() const;

    MetaPtrList importFile(const QString &filepath);
signals:
    void mediaClean();
    void scanFinished(const QString &jobid, int mediaCount);
    void meidaFileImported(const QString &jobid, MetaPtrList metalist);

public slots:
    void init();
    void removeMediaMetaList(const MetaPtrList metalist);
    void importMedias(const QString &jobid, const QStringList &urllist);

private:
    explicit MediaLibrary(QObject *parent = 0);
    friend class DMusic::DSingleton<MediaLibrary>;
    QScopedPointer<MediaLibraryPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MediaLibrary)
};

