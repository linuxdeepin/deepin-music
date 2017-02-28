#pragma once

#include <QObject>
#include "plugininterface.h"

#include <mediameta.h>
#include <searchmeta.h>

class MediaMeta;

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
    void lyricLoaded(const MetaPtr meta,
                     const DMusic::SearchMeta &search,
                     const QByteArray &lyricData);
    void coverLoaded(const MetaPtr meta,
                     const DMusic::SearchMeta &search,
                     const QByteArray &coverData);

    void metaSearchFinished(const MetaPtr meta);
    void doSearchMeta(const MetaPtr meta);

    void contextSearchFinished(const QString &context, const QList<DMusic::SearchMeta> &metalist);
    void doSearchContext(const QString &context);

public:
    virtual QObject *getObject() = 0 ;
    virtual void searchMeta(const MetaPtr meta) = 0;
};

}
}
