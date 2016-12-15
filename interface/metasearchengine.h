#pragma once

#include <QObject>
#include "plugininterface.h"

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
    void searchFinished(const MusicMeta &meta);

public:
    virtual void searchMeta(const MusicMeta &info) = 0;
};

}
}
