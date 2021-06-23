/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "metadetector.h"

#include <QDebug>

#include <QTextCodec>
#include <QLocale>
#include <QTime>
#include <QFileInfo>
#include <QHash>
#include <QBuffer>
#include <QByteArray>
#include <QDir>
#include <QString>

//#ifndef DISABLE_LIBAV
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif // __cplusplus
//#endif // DISABLE_LIBAV

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>

#include <unicode/ucnv.h>

#include "util/encodingdetector.h"
#include "util/pinyinsearch.h"
#include "ffmpegdynamicinstance.h"

static QMap<QString, QByteArray> localeCodes;


typedef void (*register_all_function)(void);
typedef AVFormatContext *(*format_alloc_context_function)(void);
typedef int (*format_open_input_function)(AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
typedef void (*format_free_context_function)(AVFormatContext *);
typedef int (*format_find_stream_info_function)(AVFormatContext *, AVDictionary **);
typedef int (*find_best_stream_function)(AVFormatContext *,
                                         enum AVMediaType,
                                         int,
                                         int,
                                         AVCodec **,
                                         int);
typedef void (*format_close_input_function)(AVFormatContext **);
typedef AVCodecContext *(*codec_alloc_context3_function)(const AVCodec *);
typedef int (*codec_parameters_to_context_function)(AVCodecContext *,
                                                    const AVCodecParameters *);
typedef AVCodec *(*codec_find_decoder_function)(enum AVCodecID);
typedef int (*codec_open2_function)(AVCodecContext *, const AVCodec *, AVDictionary **);
typedef AVPacket *(*packet_alloc_function)(void);
typedef AVFrame *(*frame_alloc_function)(void);
typedef int (*read_frame_function)(AVFormatContext *, AVPacket *);
typedef void (*packet_unref_function)(AVPacket *);
typedef void (*frame_free_function)(AVFrame **);
typedef int (*codec_close_function)(AVCodecContext *);
typedef int (*codec_send_packet_function)(AVCodecContext *, const AVPacket *);
typedef int (*codec_receive_frame_function)(AVCodecContext *, AVFrame *);

void MetaDetector::init()
{
    register_all_function register_all = FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_register_all");
    register_all();
    localeCodes.insert("zh_CN", "GB18030");
}

#if QT_VERSION >= 0x040000
# define QStringToTString(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8)
#else
# define QStringToTString(s) TagLib::String(s.utf8().data(), TagLib::String::UTF8)
#endif

QList<QByteArray> MetaDetector::detectEncodings(const QByteArray &rawData)
{
    return DMusic::EncodingDetector::detectEncodings(rawData);;
}

void MetaDetector::updateCueFileTagCodec(MediaMeta &meta, const QFileInfo &/*cueFi*/, const QByteArray &codec)
{
//    DMusic::CueParser cueParser(meta.cuePath, codec);
//    // TODO: parse may be failed for diff code
//    for (auto cueMeta : cueParser.metalist()) {
//        if (meta.hash == cueMeta.hash) {
//            meta.title = cueMeta.title;
//            meta.singer = cueMeta.singer;
//            meta.album = cueMeta.album;
//        }
//    }
}

QList<QByteArray> MetaDetector::detectEncodings(const MediaMeta &meta)
{
    if (meta.localPath.isEmpty()) {
        return QList<QByteArray>() << "UTF-8";
    }
    QByteArray                  detectByte;

    if (!meta.cuePath.isEmpty()) {
        QFile cueFile(meta.cuePath);
        if (cueFile.open(QIODevice::ReadOnly)) {
            detectByte =  cueFile.readAll();
            return detectEncodings(detectByte);
        }
    }

#ifdef _WIN32
    TagLib::FileRef f(meta.localPath.toStdWString().c_str());
#else
    TagLib::FileRef f(meta.localPath.toStdString().c_str());
#endif
    TagLib::Tag *tag = f.tag();

    if (tag) {
        detectByte += tag->title().toCString();
        detectByte += tag->artist().toCString();
        detectByte += tag->album().toCString();
    }

    return detectEncodings(detectByte);
}

MediaMeta MetaDetector::updateMetaFromLocalfile(MediaMeta meta, const QFileInfo &fileInfo)
{
    meta.localPath = fileInfo.absoluteFilePath();
    if (meta.localPath.isEmpty()) {
        return meta;
    }
    meta.length = 0;

    meta = updateMediaFileTagCodec(meta, "", false);

    if (meta.length == 0) {
        //#ifndef DISABLE_LIBAV
        format_alloc_context_function format_alloc_context = (format_alloc_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_alloc_context", true);
        format_open_input_function format_open_input = (format_open_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_open_input", true);
        format_find_stream_info_function format_find_stream_info = (format_find_stream_info_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_find_stream_info", true);
        format_close_input_function format_close_input = (format_close_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_close_input", true);
        format_free_context_function format_free_context = (format_free_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_free_context", true);
        AVFormatContext *pFormatCtx = format_alloc_context();
        format_open_input(&pFormatCtx, meta.localPath.toStdString().c_str(), nullptr, nullptr);
        if (pFormatCtx) {
            format_find_stream_info(pFormatCtx, nullptr);
            int64_t duration = pFormatCtx->duration / 1000;
            if (duration > 0) {
                meta.length = duration;
            }
        }
        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
        //#endif // DISABLE_LIBAV
    }

    meta.size = fileInfo.size();

    auto current = QDateTime::currentDateTime();
    // HACK how to sort by add time
    meta.timestamp = current.toMSecsSinceEpoch() * 1000;
    meta.filetype = fileInfo.suffix().toLower(); //歌曲文件后缀使用小写保存

    if (meta.title.isEmpty()) {
        meta.title = fileInfo.completeBaseName();
    }

    return meta;
}

MediaMeta MetaDetector::updateMediaFileTagCodec(MediaMeta &meta, const QByteArray &codecName, bool forceEncode)
{
    if (meta.localPath.isEmpty()) {
        qCritical() << "meta localPath is empty:" << meta.title << meta.hash;
        return meta;
    }

    QByteArray detectByte;
    QByteArray detectCodec = codecName;
    auto mediaPath = QStringToTString(meta.localPath);
#ifdef _WIN32
    TagLib::FileRef f(meta->localPath.toStdWString().c_str());
#else
    TagLib::FileRef f(meta.localPath.toStdString().c_str());
#endif
    TagLib::Tag *tag = f.tag();

    if (!f.file()) {
        qCritical() << "TagLib: open file failed:" << meta.localPath << f.file();
    }

    if (!tag) {
        qWarning() << "TagLib: no tag for media file" << meta.localPath;
        return meta;
    }

    TagLib::AudioProperties *t_audioProperties = f.audioProperties();
    if (t_audioProperties == nullptr)
        return meta;
    meta.length = t_audioProperties->length() * 1000;

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
            auto allDetectCodecs = detectEncodings(detectByte);
            auto localeCode = localeCodes.value(QLocale::system().name());
            for (int i = 0; i < allDetectCodecs.size(); i++) {
                auto curDetext = allDetectCodecs[i];
                if (curDetext == "Big5" || curDetext == localeCode) {
                    detectCodec = curDetext;
                    break;
                }
            }

            if (detectCodec.isEmpty())
                detectCodec = allDetectCodecs.value(0);

            QString curStr = QString::fromLocal8Bit(tag->title().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->artist().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->album().toCString());

            for (int i = 0; i < curStr.size(); i++) {
                auto ch = curStr[i];
                if (DMusic::PinyinSearch::isChinese(ch)) {
                    detectCodec = "GB18030";
                    break;
                }
            }
        }

        QString detectCodecStr(detectCodec);
        if (detectCodecStr.compare("utf-8", Qt::CaseInsensitive) == 0) {
            meta.album = TStringToQString(tag->album());
            meta.singer = TStringToQString(tag->artist());
            meta.title = TStringToQString(tag->title());
            meta.codec = "UTF-8";  //info codec


        } else {
            QTextCodec *codec = QTextCodec::codecForName(detectCodec);
            if (codec == nullptr) {
                meta.album = TStringToQString(tag->album());
                meta.singer = TStringToQString(tag->artist());
                meta.title = TStringToQString(tag->title());
            } else {
                meta.album = codec->toUnicode(tag->album().toCString());
                meta.singer = codec->toUnicode(tag->artist().toCString());
                meta.title = codec->toUnicode(tag->title().toCString());
            }
            meta.codec = detectCodec;
        }
    } else {
        meta.album = TStringToQString(tag->album());
        meta.singer = TStringToQString(tag->artist());
        meta.title = TStringToQString(tag->title());
        meta.codec = "UTF-8";
    }

    if (meta.title.isEmpty()) {
        QFileInfo localFi(meta.localPath);
        meta.title = localFi.completeBaseName();
    }

    //empty str
    meta.album = meta.album.simplified();
    meta.singer = meta.singer.simplified();
    meta.title = meta.title.simplified();
    return meta;
}

void MetaDetector::getCoverData(const QString &path, const QString &tmpPath, const QString &hash)
{
    QString imagesDirPath = tmpPath + "/images";
    QString imageName = hash + ".jpg";
    QDir imageDir(imagesDirPath);
    if (!imageDir.exists()) {
        bool isExists = imageDir.cdUp();
        isExists &= imageDir.mkdir("images");
        isExists &= imageDir.cd("images");
    }

    QByteArray byteArray;
    if (!tmpPath.isEmpty() && !hash.isEmpty()) {
        if (imageDir.exists(imageName)) {
            QImage image(imagesDirPath + "/" + imageName);
            if (!image.isNull()) {
                return;
            }
        }
    }

//#ifndef DISABLE_LIBAV
    if (!path.isEmpty()) {
        format_alloc_context_function format_alloc_context = (format_alloc_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_alloc_context", true);
        format_open_input_function format_open_input = (format_open_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_open_input", true);
        format_close_input_function format_close_input = (format_close_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_close_input", true);
        format_free_context_function format_free_context = (format_free_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_free_context", true);

        AVFormatContext *pFormatCtx = format_alloc_context();
        format_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

        QImage image;
        if (pFormatCtx) {
            if (pFormatCtx->iformat != nullptr && pFormatCtx->iformat->read_header(pFormatCtx) >= 0) {
                for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
                    if (pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                        AVPacket pkt = pFormatCtx->streams[i]->attached_pic;
                        image = QImage::fromData(static_cast<uchar *>(pkt.data), pkt.size);
                        break;
                    }
                }
            }
        }
        if (!image.isNull()) {
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "jpg");
            image = image.scaled(QSize(200, 200), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            image.save(imagesDirPath + "/" + imageName);
        } else {
//            image = QImage(":/common/image/cover_max.svg");
//            image = image.scaled(QSize(160, 160));
//            image.save(imagesDirPath + "/" + imageName);
        }

        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
    }
//#endif // DISABLE_LIBAV
    return;
}
// 获取音乐封面图片原图
QPixmap MetaDetector::getCoverDataPixmap(MediaMeta meta)
{
    format_alloc_context_function format_alloc_context = (format_alloc_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_alloc_context", true);
    format_open_input_function format_open_input = (format_open_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_open_input", true);
    format_close_input_function format_close_input = (format_close_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_close_input", true);
    format_free_context_function format_free_context = (format_free_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_free_context", true);

    AVFormatContext *pFormatCtx = format_alloc_context();
    format_open_input(&pFormatCtx, meta.localPath.toUtf8().data(), nullptr, nullptr);

    QPixmap pixmap;
    QImage image;
    if (pFormatCtx) {
        if (pFormatCtx->iformat != nullptr && pFormatCtx->iformat->read_header(pFormatCtx) >= 0) {
            for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
                if (pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                    AVPacket pkt = pFormatCtx->streams[i]->attached_pic;
                    image = QImage::fromData(static_cast<uchar *>(pkt.data), pkt.size);
                    break;
                }
            }
        }
    }

    format_close_input(&pFormatCtx);
    format_free_context(pFormatCtx);
    pixmap = QPixmap::fromImage(image);
    return pixmap;
}

MetaDetector::MetaDetector()
{
    init();
}

//QVector<float> MetaDetector::getMetaData(const QString &path)
//{
//    QVector<float> curData;
//    if (path.isEmpty())
//        return curData;
//    format_alloc_context_function format_alloc_context = (format_alloc_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_alloc_context", true);
//    format_open_input_function format_open_input = (format_open_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_open_input", true);
//    format_find_stream_info_function format_find_stream_info = (format_find_stream_info_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_find_stream_info", true);
//    find_best_stream_function find_best_stream = (find_best_stream_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_find_best_stream", true);
//    codec_alloc_context3_function codec_alloc_context3 = (codec_alloc_context3_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_alloc_context3", true);
//    codec_parameters_to_context_function codec_parameters_to_context = (codec_parameters_to_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_parameters_to_context", true);
//    codec_find_decoder_function codec_find_decoder = (codec_find_decoder_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_find_decoder", true);
//    codec_open2_function codec_open2 = (codec_open2_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_open2", true);
//    packet_alloc_function packet_alloc = (packet_alloc_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_packet_alloc", true);
//    frame_alloc_function frame_alloc = (frame_alloc_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_frame_alloc", true);
//    read_frame_function read_frame = (read_frame_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_read_frame", true);
//    codec_send_packet_function codec_send_packet = (codec_send_packet_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_send_packet", true);
//    packet_unref_function packet_unref = (packet_unref_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_packet_unref", true);
//    codec_receive_frame_function codec_receive_frame = (codec_receive_frame_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_receive_frame", true);
//    frame_free_function frame_free = (frame_free_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_frame_free", true);
//    codec_close_function codec_close = (codec_close_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_close", true);
//    format_close_input_function format_close_input = (format_close_input_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_close_input", true);
//    format_free_context_function format_free_context = (format_free_context_function)FfmpegDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_free_context", true);

//    AVFormatContext *pFormatCtx = format_alloc_context();
//    format_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

//    if (pFormatCtx == nullptr)
//        return curData;

//    format_find_stream_info(pFormatCtx, nullptr);

//    int audio_stream_index = -1;
//    audio_stream_index = find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
//    if (audio_stream_index < 0)
//        return curData;

//    AVStream *in_stream = pFormatCtx->streams[audio_stream_index];
//    AVCodecParameters *in_codecpar = in_stream->codecpar;

//    AVCodecContext *pCodecCtx = codec_alloc_context3(nullptr);
//    codec_parameters_to_context(pCodecCtx, in_codecpar);

//    AVCodec *pCodec = codec_find_decoder(pCodecCtx->codec_id);
//    codec_open2(pCodecCtx, pCodec, nullptr);

//    AVPacket *packet = packet_alloc();
//    AVFrame *frame = frame_alloc();

//    while (read_frame(pFormatCtx, packet) >= 0) {
//        if (packet->stream_index == audio_stream_index) {

//            int state;
//            state = codec_send_packet(pCodecCtx, packet);
//            packet_unref(packet);
//            if (state != 0) {
//                continue;
//            }

//            state = codec_receive_frame(pCodecCtx, frame);
//            if (state == 0) {

//                quint8 *ptr = frame->extended_data[0];
//                if (path.endsWith(".ape") || path.endsWith(".APE")) {
//                    for (int i = 0; i < frame->linesize[0]; i++) {
//                        auto  valDate = ((ptr[i]) << 16 | (ptr[i + 1]));
//                        curData.append(valDate + qrand());
//                    }
//                } else {
//                    for (int i = 0; i < frame->linesize[0]; i += 1024) {
//                        auto  valDate = ((ptr[i]) << 16 | (ptr[i + 1]));
//                        curData.append(valDate);
//                    }
//                }
//            }
//        }
//        packet_unref(packet);
//    }

//    packet_unref(packet);
//    frame_free(&frame);
//    codec_close(pCodecCtx);
//    format_close_input(&pFormatCtx);
//    format_free_context(pFormatCtx);

//    return curData;
//}
