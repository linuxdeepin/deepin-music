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
//        QFile::copy(path, fromPath);
        QString program = QString("ffmpeg -i %1 -ac 1 -ab 32 -ar 24000 %2/.tmp.ape.mp3").arg(fromPath).arg(curPath);
        QProcess::execute(program);
        path = toPath;
    }
#endif
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, path.toStdString().c_str(), NULL, NULL);

    if (pFormatCtx == nullptr) {
        avformat_free_context(pFormatCtx);
        return;
    }

    avformat_find_stream_info(pFormatCtx, NULL);

    int audio_stream_index = -1;
    audio_stream_index = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_stream_index < 0) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        return;
    }

    AVStream *in_stream = pFormatCtx->streams[audio_stream_index];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx, in_codecpar);

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    avcodec_open2(pCodecCtx, pCodec, NULL);

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    QVector<float> curData;
    bool flag = false;
    while (av_read_frame(pFormatCtx, packet) >= 0 ) {
        //stop detector
        if (d->stopFlag && curData.size() > 100) {
            av_packet_unref(packet);
            av_frame_free(&frame);
            avcodec_close(pCodecCtx);
            avformat_close_input(&pFormatCtx);
            avformat_free_context(pFormatCtx);
            resample(curData, hash);//刷新波浪条
            d->stopFlag = false;
            return;
        }

        if (!flag && curData.size() > 100) {
            resample(curData, hash);
            flag = true;
        }
        if (packet->stream_index == audio_stream_index) {
            int got_picture;
            uint32_t ret = avcodec_decode_audio4( pCodecCtx, frame, &got_picture, packet);
            if ( ret < 0 ) {
                continue;
            }
            if ( got_picture > 0 ) {
                int data_size = av_get_bytes_per_sample(pCodecCtx->sample_fmt);
                int t_format = frame->format;
                uint8_t *ptr = frame->extended_data[0];
                short val;
                if (path.endsWith(".ape") || path.endsWith(".APE")) {
                    for (int i = 0; i < frame->linesize[0]; i++) {
                        val = (short)(
                                  ((unsigned char)ptr[i]) << 16 |
                                  ((unsigned char)ptr[i + 1])
                              );
                            curData.append(val+qrand());
                    }
                }else {
                    for (int i = 0; i < frame->linesize[0]; i += 1024) {
                        val = (short)(
                                  ((unsigned char)ptr[i]) << 8 |
                                  ((unsigned char)ptr[i + 1])
                              );
                        curData.append(val);
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
