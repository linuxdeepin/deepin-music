#pragma once

#include "util/singleton.h"

#include <mediameta.h>

class QFileInfo;
class LIBDMUSICSHARED_EXPORT MetaDetector
{
public:
    static void init();

    static QList<QByteArray> detectEncodings(const MetaPtr meta);
    static QList<QByteArray> detectEncodings(const QByteArray &rawData);

    static void updateCueFileTagCodec(MediaMeta *meta, const QFileInfo &, const QByteArray &codec);
    static void updateMediaFileTagCodec(MediaMeta *meta, const QByteArray &codec, bool forceEncode);
    static void updateMetaFromLocalfile(MediaMeta *meta, const QFileInfo &fileInfo);
};
