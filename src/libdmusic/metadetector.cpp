/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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
#include "util/cueparser.h"
#include "util/pinyinsearch.h"

static QMap<QString, QByteArray> localeCodes;

void MetaDetector::init()
{
//#ifndef DISABLE_LIBAV
    av_register_all();
//#endif // DISABLE_LIBAV
    localeCodes.insert("zh_CN", "GB18030");
}

#if QT_VERSION >= 0x040000
# define QStringToTString(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8)
#else
# define QStringToTString(s) TagLib::String(s.utf8().data(), TagLib::String::UTF8)
#endif

QList<QByteArray> MetaDetector::detectEncodings(const QByteArray &rawData)
{
    auto icuCodes = DMusic::EncodingDetector::detectEncodings(rawData);
//    auto localeCode = localeCodes.value(QLocale::system().name());

//    if (icuCodes.contains(localeCode)) {
//        icuCodes.removeAll(localeCode);
//    }

//    if (!localeCode.isEmpty()) {
//        icuCodes.push_front(localeCode);
//    }
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

    TagLib::AudioProperties *t_audioProperties = f.audioProperties();
    if (t_audioProperties == nullptr)
        return ;
    meta->length = t_audioProperties->length() * 1000;

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

            for (auto curDetext : allDetectCodecs) {
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
            for (auto ch : curStr) {
                if (DMusic::PinyinSearch::isChinese(ch)) {
                    detectCodec = "GB18030";
                    break;
                }
            }
        }

        QString detectCodecStr(detectCodec);
        if (detectCodecStr.compare("utf-8", Qt::CaseInsensitive) == 0) {
            meta->album = TStringToQString(tag->album());
            meta->artist = TStringToQString(tag->artist());
            meta->title = TStringToQString(tag->title());
            meta->codec = "UTF-8";  //info codec


        } else {
            QTextCodec *codec = QTextCodec::codecForName(detectCodec);
            if (codec == nullptr) {
                meta->album = TStringToQString(tag->album());
                meta->artist = TStringToQString(tag->artist());
                meta->title = TStringToQString(tag->title());
            } else {
                meta->album = codec->toUnicode(tag->album().toCString());
                meta->artist = codec->toUnicode(tag->artist().toCString());
                meta->title = codec->toUnicode(tag->title().toCString());
            }
            meta->codec = detectCodec;
        }

#ifdef true
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
#endif
    } else {
        meta->album = TStringToQString(tag->album());
        meta->artist = TStringToQString(tag->artist());
        meta->title = TStringToQString(tag->title());
        meta->codec = "UTF-8";
    }

    if (meta->title.isEmpty()) {
        QFileInfo localFi(meta->localPath);
        meta->title = localFi.completeBaseName();
    }

    //empty str
    meta->album = meta->album.simplified();
    meta->artist = meta->artist.simplified();
    meta->title = meta->title.simplified();
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

    updateMediaFileTagCodec(meta, "", false);

    if (meta->length == 0) {
        //#ifndef DISABLE_LIBAV
        AVFormatContext *pFormatCtx = avformat_alloc_context();
        avformat_open_input(&pFormatCtx, meta->localPath.toStdString().c_str(), nullptr, nullptr);
        if (pFormatCtx) {
            avformat_find_stream_info(pFormatCtx, nullptr);
            int64_t duration = pFormatCtx->duration / 1000;
            if (duration > 0) {
                meta->length = duration;
            }
        }
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        //#endif // DISABLE_LIBAV
    }

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

QByteArray MetaDetector::getCoverData(const QString &path, const QString &tmpPath, const QString &hash)
{
    QString imagesDirPath = tmpPath + "/images";
    QString imageName = hash + ".jpg";
    QDir imageDir(imagesDirPath);
    if (!imageDir.exists()) {
        imageDir.cdUp();
        imageDir.mkdir("images");
        imageDir.cd("images");
    }

    QByteArray byteArray;
    if (!tmpPath.isEmpty() && !hash.isEmpty()) {
        if (imageDir.exists(imageName)) {
            QImage image(imagesDirPath + "/" + imageName);
            if (!image.isNull()) {
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "jpg");
            }
            return byteArray;
        }
    }

//#ifndef DISABLE_LIBAV
    if (!path.isEmpty()) {
        AVFormatContext *pFormatCtx = avformat_alloc_context();
        avformat_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

        QImage image;
        if (pFormatCtx) {
            if (pFormatCtx->iformat != nullptr && pFormatCtx->iformat->read_header(pFormatCtx) >= 0) {
                int index = static_cast<int>(pFormatCtx->nb_streams);
                for (int i = 0; i < index; i++) {
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
            image = image.scaled(QSize(160, 160));
            image.save(imagesDirPath + "/" + imageName);
        } else {
            image = QImage(":/common/image/cover_max.svg");
            image = image.scaled(QSize(160, 160));
            image.save(imagesDirPath + "/" + imageName);
        }

        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }
//#endif // DISABLE_LIBAV

    return byteArray;
}

QVector<float> MetaDetector::getMetaData(const QString &path)
{
    QVector<float> curData;
    if (path.isEmpty())
        return curData;

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

    if (pFormatCtx == nullptr)
        return curData;

    avformat_find_stream_info(pFormatCtx, nullptr);

    int audio_stream_index = -1;
    audio_stream_index = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_stream_index < 0)
        return curData;

    AVStream *in_stream = pFormatCtx->streams[audio_stream_index];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(pCodecCtx, in_codecpar);

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    avcodec_open2(pCodecCtx, pCodec, nullptr);

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audio_stream_index) {

            int state;
            state = avcodec_send_packet(pCodecCtx, packet);
            av_packet_unref(packet);
            if (state != 0) {
                continue;
            }

            state = avcodec_receive_frame(pCodecCtx, frame);
            if (state == 0) {

                quint8 *ptr = frame->extended_data[0];
                if (path.endsWith(".ape") || path.endsWith(".APE")) {
                    for (int i = 0; i < frame->linesize[0]; i++) {
                        auto  valDate = ((ptr[i]) << 16 | (ptr[i + 1]));
                        curData.append(valDate + qrand());
                    }
                } else {
                    for (int i = 0; i < frame->linesize[0]; i += 1024) {
                        auto  valDate = ((ptr[i]) << 16 | (ptr[i + 1]));
                        curData.append(valDate);
                    }
                }
            }
        }
        av_packet_unref(packet);
    }

    av_packet_unref(packet);
    av_frame_free(&frame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

    return curData;
}
