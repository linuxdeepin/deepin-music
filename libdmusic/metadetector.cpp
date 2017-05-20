#include "metadetector.h"

#include <QDebug>

#include <QTextCodec>
#include <QLocale>
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

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>

#include <unicode/ucnv.h>

#include "util/encodingdetector.h"
#include "util/cueparser.h"

static QMap<QString, QByteArray> localeCodes;

void MetaDetector::init()
{
    av_register_all();
    localeCodes.insert("zh_CN", "GB18030");
}


QList<QByteArray> MetaDetector::detectEncodings(const QByteArray &rawData)
{
    auto icuCodes = DMusic::EncodingDetector::detectEncodings(rawData);
    auto localeCode = localeCodes.value(QLocale::system().name());

    if (icuCodes.contains(localeCode)) {
        icuCodes.removeAll(localeCode);
    }

    if (!localeCode.isEmpty()) {
        icuCodes.push_front(localeCode);
    }
    return icuCodes;
}

void MetaDetector::updateCueFileTagCodec(MediaMeta *meta, const QFileInfo &/*cueFi*/, const QByteArray &codec)
{
    Q_ASSERT(meta != nullptr);
    DMusic::CueParser cueParser(meta->cuePath, codec);
    // TODO: parse may be failed for diff code
    for (auto cueMeta : cueParser.metalist()) {
        if (meta->hash == cueMeta->hash) {
            meta->title = cueMeta->title;
            meta->artist = cueMeta->artist;
            meta->album = cueMeta->album;
        }
    }
}

void MetaDetector::updateMediaFileTagCodec(MediaMeta *meta, const QByteArray &codecName, bool forceEncode)
{
    Q_ASSERT(meta != nullptr);

    if (meta->localPath.isEmpty()) {
        qCritical() << "meta localPath is empty:" << meta->title << meta->hash;
        return ;
    }

    QByteArray detectByte;
    QByteArray detectCodec = codecName;
    auto mediaPath = QStringToTString(meta->localPath);
#ifdef _WIN32
    TagLib::FileRef f(meta->localPath.toStdWString().c_str());
#else
    TagLib::FileRef f(meta->localPath.toStdString().c_str());
#endif
    TagLib::Tag *tag = f.tag();

    if (!f.file()) {
        qCritical() << "TagLib: open file failed:" << meta->localPath << f.file();
    }

    if (!tag) {
        qWarning() << "TagLib: no tag for media file" << meta->localPath;
        return;
    }

    bool encode = true;
    encode &= tag->title().isNull() ? true : tag->title().isLatin1();
    encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
    encode &= tag->album().isNull() ? true : tag->album().isLatin1();

    if (forceEncode) {
        encode = true;
    }

    if (encode) {
        if (detectCodec.isEmpty()) {
            detectByte += tag->title().toCString();
            detectByte += tag->artist().toCString();
            detectByte += tag->album().toCString();
            detectCodec = detectEncodings(detectByte).value(0);
//            qDebug() << "detect codec" << detectEncodings(detectByte);
        }

//        qDebug() << "convert to" << detectCodec;
//        QTextCodec *codec = QTextCodec::codecForName(detectCodec);
//        meta->album = codec->toUnicode(tag->album().to8Bit().c_str());
//        meta->artist = codec->toUnicode(tag->artist().to8Bit().c_str());
//        meta->title = codec->toUnicode(tag->title().to8Bit().c_str());

//#ifndef true
//        qDebug() << "convert to" << detectCodec << QTextCodec::availableCodecs();
        const size_t buflen = 1024 * 10;
        char buf[buflen];
        UErrorCode err = U_ZERO_ERROR;
        int32_t len = ucnv_convert("utf-8", detectCodec, buf, buflen, tag->title().toCString(), -1, &err);
        meta->title = QString::fromUtf8(buf);
//        qDebug() << len <<  QString::fromUtf8(buf) << buf << u_errorName(err);

        err = U_ZERO_ERROR;
        len = ucnv_convert("utf-8", detectCodec, buf, buflen, tag->artist().toCString(), -1, &err);
        meta->artist = QString::fromUtf8(buf);
//        qDebug() << len <<  QString::fromUtf8(buf) << buf << u_errorName(err);

        err = U_ZERO_ERROR;
        len = ucnv_convert("utf-8", detectCodec, buf, buflen, tag->album().toCString(), -1, &err);
        meta->album = QString::fromUtf8(buf);
//        qDebug() << len <<  QString::fromUtf8(buf) << buf << u_errorName(err);
        Q_UNUSED(len);
//#endif
    } else {
        meta->album = TStringToQString(tag->album());
        meta->artist = TStringToQString(tag->artist());
        meta->title = TStringToQString(tag->title());
    }

    if (meta->title.isEmpty()) {
        QFileInfo localFi(meta->localPath);
        meta->title = localFi.completeBaseName();
    }
}

QList<QByteArray> MetaDetector::detectEncodings(const MetaPtr meta)
{
    if (meta->localPath.isEmpty()) {
        return QList<QByteArray>() << "UTF-8";
    }
    QByteArray                  detectByte;

    if (!meta->cuePath.isEmpty()) {
        QFile cueFile(meta->cuePath);
        if (cueFile.open(QIODevice::ReadOnly)) {
            detectByte =  cueFile.readAll();
            return detectEncodings(detectByte);
        }
    }

#ifdef _WIN32
    TagLib::FileRef f(meta->localPath.toStdWString().c_str());
#else
    TagLib::FileRef f(meta->localPath.toStdString().c_str());
#endif
    TagLib::Tag *tag = f.tag();

    if (tag) {
        detectByte += tag->title().toCString();
        detectByte += tag->artist().toCString();
        detectByte += tag->album().toCString();
    }

    return detectEncodings(detectByte);
}

void MetaDetector::updateMetaFromLocalfile(MediaMeta *meta, const QFileInfo &fileInfo)
{
    Q_ASSERT(meta != nullptr);

    meta->localPath = fileInfo.absoluteFilePath();
    if (meta->localPath.isEmpty()) {
        return ;
    }
    meta->length = 0;
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, meta->localPath.toStdString().c_str(), NULL, NULL);
    if (pFormatCtx) {
        avformat_find_stream_info(pFormatCtx, NULL);
        int64_t duration = pFormatCtx->duration / 1000;
        if (duration > 0) {
            meta->length = duration;
        }
    }
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

    updateMediaFileTagCodec(meta, "", false);

    meta->size = fileInfo.size();

    auto current = QDateTime::currentDateTime();
    // HACK how to sort by add time
    meta->timestamp = current.toMSecsSinceEpoch() * 1000;
    meta->filetype = fileInfo.suffix();

    if (meta->title.isEmpty()) {
        meta->title = fileInfo.completeBaseName();
    }

    meta->updateSearchIndex();
}
