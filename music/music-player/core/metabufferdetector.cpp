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

#include "metabufferdetector.h"

#include <QDebug>

#include <QTextCodec>
#include <QLocale>
#include <QTime>
#include <QFileInfo>
#include <QHash>
#include <QBuffer>
#include <QByteArray>
#include <QProcess>

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

#include "util/global.h"
#include "core/vlc/vlcdynamicinstance.h"


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

class MetaBufferDetectorPrivate
{
public:
    MetaBufferDetectorPrivate(MetaBufferDetector *parent) : q_ptr(parent) {}
    QString           curPath;
    QString           curHash;
    QVector<float>    listData;

    bool              stopFlag = false;

    MetaBufferDetector *q_ptr;
    Q_DECLARE_PUBLIC(MetaBufferDetector)
};

MetaBufferDetector::MetaBufferDetector(QObject *parent)
    : QThread(parent), d_ptr(new MetaBufferDetectorPrivate(this))
{

}

MetaBufferDetector::~MetaBufferDetector()
{
    Q_D(MetaBufferDetector);
    d->stopFlag = true;
    while (isRunning()) {

    }
}

void MetaBufferDetector::run()
{
    Q_D(MetaBufferDetector);
    QString path = d->curPath;
    QString hash = d->curHash;
    if (path.isEmpty())
        return;
#if 0
    QFileInfo fileInfo(path);
    if (fileInfo.suffix() == "ape") {
        QString curPath = Global::configPath();
        QString toPath = QString("%1/.tmp.ape.mp3").arg(curPath);
        if (QFile::exists(toPath)) {
            QFile::remove(toPath);
        }
        QString fromPath = QString("%1/.tmp.ape").arg(curPath);
        QFile file(path);
        file.link(fromPath);
        QString program = QString("ffmpeg -i %1 -ac 1 -ab 32 -ar 24000 %2/.tmp.ape.mp3").arg(fromPath).arg(curPath);
        QProcess::execute(program);
        path = toPath;
    }
#endif
    format_alloc_context_function format_alloc_context = (format_alloc_context_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_alloc_context", true);
    format_open_input_function format_open_input = (format_open_input_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_open_input", true);
    format_free_context_function format_free_context = (format_free_context_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_free_context", true);
    format_find_stream_info_function format_find_stream_info = (format_find_stream_info_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_find_stream_info", true);
    find_best_stream_function find_best_stream = (find_best_stream_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_find_best_stream", true);
    format_close_input_function format_close_input = (format_close_input_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avformat_close_input", true);
    codec_alloc_context3_function codec_alloc_context3 = (codec_alloc_context3_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_alloc_context3", true);
    codec_parameters_to_context_function codec_parameters_to_context = (codec_parameters_to_context_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_parameters_to_context", true);
    codec_find_decoder_function codec_find_decoder = (codec_find_decoder_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_find_decoder", true);
    codec_open2_function codec_open2 = (codec_open2_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_open2", true);
    packet_alloc_function packet_alloc = (packet_alloc_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_packet_alloc", true);
    frame_alloc_function frame_alloc = (frame_alloc_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_frame_alloc", true);
    read_frame_function read_frame = (read_frame_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_read_frame", true);

    packet_unref_function packet_unref = (packet_unref_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_packet_unref", true);
    frame_free_function frame_free = (frame_free_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("av_frame_free", true);
    codec_close_function codec_close = (codec_close_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_close", true);
    codec_send_packet_function codec_send_packet = (codec_send_packet_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_send_packet", true);
    codec_receive_frame_function codec_receive_frame = (codec_receive_frame_function)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("avcodec_receive_frame", true);

    AVFormatContext *pFormatCtx = format_alloc_context();
    format_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

    if (pFormatCtx == nullptr) {
        format_free_context(pFormatCtx);
        d->curPath.clear();
        d->curHash.clear();
        return;
    }

    format_find_stream_info(pFormatCtx, nullptr);

    int audio_stream_index = -1;
    audio_stream_index = find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_stream_index < 0) {
        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
        d->curPath.clear();
        d->curHash.clear();
        return;
    }

    AVStream *in_stream = pFormatCtx->streams[audio_stream_index];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    AVCodecContext *pCodecCtx = codec_alloc_context3(nullptr);
    codec_parameters_to_context(pCodecCtx, in_codecpar);

    AVCodec *pCodec = codec_find_decoder(pCodecCtx->codec_id);
    codec_open2(pCodecCtx, pCodec, nullptr);

    AVPacket *packet = packet_alloc();
    AVFrame *frame = frame_alloc();

    QVector<float> curData;
    bool flag = false;
    while (read_frame(pFormatCtx, packet) >= 0) {
        //stop detector
        if (d->stopFlag && curData.size() > 100) {
            packet_unref(packet);
            frame_free(&frame);
            codec_close(pCodecCtx);
            format_close_input(&pFormatCtx);
            format_free_context(pFormatCtx);
            resample(curData, hash);//刷新波浪条
            d->stopFlag = false;
            d->curPath.clear();
            d->curHash.clear();
            return;
        }

        if (!flag && curData.size() > 100) {
            resample(curData, hash);
            flag = true;
        }

        while (read_frame(pFormatCtx, packet) >= 0) {
            if (packet->stream_index == audio_stream_index) {
                int state;
                state = codec_send_packet(pCodecCtx, packet);
                packet_unref(packet);
                if (state != 0) {
                    continue;
                }

                state = codec_receive_frame(pCodecCtx, frame);
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
        }
        packet_unref(packet);
    }

    packet_unref(packet);
    frame_free(&frame);
    codec_close(pCodecCtx);
    format_close_input(&pFormatCtx);
    format_free_context(pFormatCtx);

    resample(curData, hash);
}

void MetaBufferDetector::onBufferDetector(const QString &path, const QString &hash)
{
    Q_D(MetaBufferDetector);
    QString curHash = d->curHash;
    if (hash == curHash/* || true*/)
        return;
    if (isRunning()) {
        d->stopFlag = true;
    }
    d->curPath = path;
    d->curHash = hash;
    start();
}

void MetaBufferDetector::onClearBufferDetector()
{
    Q_D(MetaBufferDetector);
    if (isRunning()) {
        d->stopFlag = true;
    }
    d->curPath.clear();
    d->curHash.clear();
}

void MetaBufferDetector::resample(const QVector<float> &buffer, const QString &hash)
{
    QVector<float> t_buffer;
    t_buffer.reserve(1001);
    if (buffer.size() < 1000) {
        t_buffer = buffer;
    } else {
        int num = buffer.size() / 1000;
        float t_curValue = 0;
        for (int i = 0; i < buffer.size(); i += num) {
            if (i % num == 0) {
                t_buffer.append(buffer[i]);
            }
        }
        t_buffer.append(t_curValue);
    }
    Q_EMIT metaBuffer(t_buffer, hash);
}
