#include "metadetector.h"

#include <QDebug>

#include <QTextCodec>
#include <QTime>
#include <QFileInfo>
#include <QHash>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif

#include "util/encodingdetector.h"
#include "util/cueparser.h"

MetaDetector::MetaDetector()
{
    qDebug() << "av_register_all";
}

void MetaDetector::updateMetaFromLocalfile(MediaMeta *meta, const QFileInfo &fileInfo)
{
    Q_ASSERT(meta != nullptr);

    meta->localPath = fileInfo.absoluteFilePath();
    if (meta->localPath.isEmpty()) {
        return ;
    }

    QMap<QString, QByteArray>   tags;
    QByteArray                  detectByte;

    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, meta->localPath.toStdString().c_str(), NULL, NULL);
    if (pFormatCtx) {
        AVDictionaryEntry *tag = NULL;
        avformat_find_stream_info(pFormatCtx, NULL);
        while ((tag = av_dict_get(pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
            qDebug() << "FFMPEG: get tag: " << tag->key << "=" << tag->value;
            tags.insert(tag->key, tag->value);
            detectByte += tag->value;
        }
        int64_t duration = pFormatCtx->duration / 1000;
        meta->length = duration;
    }

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

    QByteArray codeName = DMusic::EncodingDetector::detectEncodings(detectByte).value(0);
    QTextCodec *codec = QTextCodec::codecForName(codeName);

    if (codec) {
        meta->album = codec->toUnicode(tags.value("album"));
        meta->artist = codec->toUnicode(tags.value("artist"));
        meta->title = codec->toUnicode(tags.value("title"));
    } else {
        // UTF8 encoded.
        meta->album = tags.value("album");
        meta->artist = tags.value("artist");
        meta->title = tags.value("title");
    }

    meta->size = fileInfo.size();

    auto current = QDateTime::currentDateTime();
    // HACK how to sort by add time
    meta->timestamp = current.toMSecsSinceEpoch() * 1000;
    meta->filetype = fileInfo.suffix();

    if (meta->title.isEmpty()) {
        meta->title = fileInfo.baseName();
    }

    meta->updateSearchIndex();
}
