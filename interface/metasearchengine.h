#pragma once

#include <QObject>
#include "plugininterface.h"

#include "../music-player/core/music.h"

class MusicMeta;

namespace DMusic
{
namespace Plugin
{

class MetaSearchEngine : public QObject, public PluginInterface
{
    Q_OBJECT
public:
    explicit MetaSearchEngine(QObject *parent = 0) : QObject(parent) {}

signals:
    void lyricLoaded(const MusicMeta &meta, const QByteArray &lyricData);
    void coverLoaded(const MusicMeta &meta, const QByteArray &coverData);

    void metaSearchFinished(const MusicMeta &meta);
    void doSearchMeta(const MusicMeta &info);

    void contextSearchFinished(const QString &context, const MusicMetaList &metalist);
    void doSearchContext(const QString &context);

public:
    virtual QObject *getObject() = 0 ;
    virtual void searchMeta(const MusicMeta &info) = 0;
};

}
}
