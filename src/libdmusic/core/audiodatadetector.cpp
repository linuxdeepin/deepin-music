// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiodatadetector.h"

#include <QTextCodec>
#include <QLocale>
#include <QTime>
#include <QFileInfo>
#include <QHash>
#include <QBuffer>
#include <QByteArray>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

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

#include "dynamiclibraries.h"
#include "global.h"
#include "util/log.h"

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

AudioDataDetector::AudioDataDetector(QObject *parent)
    : QThread(parent)
{
    qCDebug(dmMusic) << "Initializing AudioDataDetector";
    connect(this, &AudioDataDetector::audioBufferFromThread, this, &AudioDataDetector::audioBuffer, Qt::QueuedConnection);
    qCDebug(dmMusic) << "AudioDataDetector initialized with queued connection";
}

AudioDataDetector::~AudioDataDetector()
{
    qCDebug(dmMusic) << "Destroying AudioDataDetector";
    m_stopFlag = true;
    while (isRunning()) {
        qCDebug(dmMusic) << "Waiting for detection thread to finish";
    }
    qCDebug(dmMusic) << "AudioDataDetector destroyed";
}

void AudioDataDetector::run()
{
    QString path = m_curPath;
    QString hash = m_curHash;
    qCInfo(dmMusic) << "Starting audio data detection for file:" << path << "hash:" << hash;
    
    if (path.isEmpty()) {
        qCWarning(dmMusic) << "Path is empty, aborting audio data detection";
        return;
    }
    
    format_alloc_context_function format_alloc_context = (format_alloc_context_function)DynamicLibraries::instance()->resolve("avformat_alloc_context", true);
    format_open_input_function format_open_input = (format_open_input_function)DynamicLibraries::instance()->resolve("avformat_open_input", true);
    format_free_context_function format_free_context = (format_free_context_function)DynamicLibraries::instance()->resolve("avformat_free_context", true);
    format_find_stream_info_function format_find_stream_info = (format_find_stream_info_function)DynamicLibraries::instance()->resolve("avformat_find_stream_info", true);
    find_best_stream_function find_best_stream = (find_best_stream_function)DynamicLibraries::instance()->resolve("av_find_best_stream", true);
    format_close_input_function format_close_input = (format_close_input_function)DynamicLibraries::instance()->resolve("avformat_close_input", true);
    codec_alloc_context3_function codec_alloc_context3 = (codec_alloc_context3_function)DynamicLibraries::instance()->resolve("avcodec_alloc_context3", true);
    codec_parameters_to_context_function codec_parameters_to_context = (codec_parameters_to_context_function)DynamicLibraries::instance()->resolve("avcodec_parameters_to_context", true);
    codec_find_decoder_function codec_find_decoder = (codec_find_decoder_function)DynamicLibraries::instance()->resolve("avcodec_find_decoder", true);
    codec_open2_function codec_open2 = (codec_open2_function)DynamicLibraries::instance()->resolve("avcodec_open2", true);
    packet_alloc_function packet_alloc = (packet_alloc_function)DynamicLibraries::instance()->resolve("av_packet_alloc", true);
    frame_alloc_function frame_alloc = (frame_alloc_function)DynamicLibraries::instance()->resolve("av_frame_alloc", true);
    read_frame_function read_frame = (read_frame_function)DynamicLibraries::instance()->resolve("av_read_frame", true);

    packet_unref_function packet_unref = (packet_unref_function)DynamicLibraries::instance()->resolve("av_packet_unref", true);
    frame_free_function frame_free = (frame_free_function)DynamicLibraries::instance()->resolve("av_frame_free", true);
    codec_close_function codec_close = (codec_close_function)DynamicLibraries::instance()->resolve("avcodec_close", true);
    codec_send_packet_function codec_send_packet = (codec_send_packet_function)DynamicLibraries::instance()->resolve("avcodec_send_packet", true);
    codec_receive_frame_function codec_receive_frame = (codec_receive_frame_function)DynamicLibraries::instance()->resolve("avcodec_receive_frame", true);

    AVFormatContext *pFormatCtx = format_alloc_context();
    int ret = format_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

    if (pFormatCtx == nullptr || ret != 0) {
        qCCritical(dmMusic) << "Failed to open input format context for file:" << path << "error code:" << ret;
        format_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    qCDebug(dmMusic) << "Successfully opened format context for file:" << path;
    ret = format_find_stream_info(pFormatCtx, nullptr);
    if (ret < 0) {
        qCWarning(dmMusic) << "Failed to find stream info for file:" << path << "error code:" << ret;
    }

    int audio_stream_index = -1;
    audio_stream_index = find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_stream_index < 0) {
        qCWarning(dmMusic) << "No audio stream found in file:" << path;
        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    qCDebug(dmMusic) << "Found audio stream at index:" << audio_stream_index << "for file:" << path;

    AVStream *in_stream = pFormatCtx->streams[audio_stream_index];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    AVCodecContext *pCodecCtx = codec_alloc_context3(nullptr);
    codec_parameters_to_context(pCodecCtx, in_codecpar);

    AVCodec *pCodec = codec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr) {
        qCCritical(dmMusic) << "Failed to find decoder for codec ID:" << pCodecCtx->codec_id << "in file:" << path;
        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }
    
    ret = codec_open2(pCodecCtx, pCodec, nullptr);
    if (ret < 0) {
        qCCritical(dmMusic) << "Failed to open codec for file:" << path << "error code:" << ret;
        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    qCDebug(dmMusic) << "Successfully initialized codec for file:" << path;

    AVPacket *packet = packet_alloc();
    AVFrame *frame = frame_alloc();

    QVector<float> curData;

    while (read_frame(pFormatCtx, packet) >= 0) {
        //stop detector
        if (m_stopFlag && curData.size() > 100) {
            qCDebug(dmMusic) << "Stop flag detected, cleaning up resources for file:" << path;
            packet_unref(packet);
            frame_free(&frame);
            codec_close(pCodecCtx);
            format_close_input(&pFormatCtx);
            format_free_context(pFormatCtx);
            resample(curData, hash, true);//刷新波浪条
            m_stopFlag = false;
            m_curPath.clear();
            m_curHash.clear();
            qCDebug(dmMusic) << "Successfully stopped detection for file:" << path;
            return;
        }

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
                    for (int i = 0; (i + 1) < frame->linesize[0]; i++) {
                        auto  valDate = ((ptr[i]) << 16 | (ptr[i + 1]));
                        //curData.append((float)valDate + qrand());
                        curData.append(static_cast<float>(valDate) + QRandomGenerator::global()->generate());
                    }
                } else {
                    for (int i = 0; (i + 1) < frame->linesize[0]; i += 1024) {
                        auto  valDate = ((ptr[i]) << 16 | (ptr[i + 1]));
                        curData.append(valDate);
                    }
                }
            }
        }
    }

    packet_unref(packet);
    frame_free(&frame);
    codec_close(pCodecCtx);
    format_close_input(&pFormatCtx);
    format_free_context(pFormatCtx);
    resample(curData, hash);
}

void AudioDataDetector::onBufferDetector(const QString &path, const QString &hash)
{
    qCDebug(dmMusic) << "Received buffer detection request for file:" << path << "hash:" << hash;
    QString curHash = m_curHash;
    if (hash == curHash/* || true*/) {
        qCDebug(dmMusic) << "Hash matches current processing hash, ignoring request:" << hash;
        return;
    }
    if (isRunning()) {
        qCDebug(dmMusic) << "Detection thread is running, setting stop flag";
        m_stopFlag = true;
    }
    m_curPath = path;
    m_curHash = hash;
    if (!queryCacheExisted(hash) && DmGlobal::playbackEngineType() == 1) { //查询到本地无缓存信息
        qCInfo(dmMusic) << "No cache found for hash:" << hash << "starting audio data detection thread";
        start();
    } else {
        qCDebug(dmMusic) << "Cache exists for hash:" << hash << "or engine type is not FFmpeg, skipping detection";
    }
}

void AudioDataDetector::onClearBufferDetector()
{
    qCDebug(dmMusic) << "Clearing buffer detector, current path:" << m_curPath << "hash:" << m_curHash;
    if (isRunning()) {
        qCDebug(dmMusic) << "Detection thread is running, setting stop flag";
        m_stopFlag = true;
    }
    m_curPath.clear();
    m_curHash.clear();
    qCDebug(dmMusic) << "Buffer detector cleared";
}

void AudioDataDetector::resample(const QVector<float> &buffer, const QString &hash, bool forceQuit)
{
    qCDebug(dmMusic) << "Resampling audio data for hash:" << hash << "buffer size:" << buffer.size() << "forceQuit:" << forceQuit;
    
    if (buffer.isEmpty()) {
        qCWarning(dmMusic) << "Buffer is empty, cannot resample for hash:" << hash;
        qDebug() << __FUNCTION__ << "buffer size ==" << buffer.size();
        return;
    }

    QVector<float> t_buffer;
    QVector<float> s_buffer;
    QVector<float> mappingbuf;
    t_buffer.reserve(1001);
    if (buffer.size() < 1000) {
        t_buffer = buffer;
        qCDebug(dmMusic) << "Buffer size is small, using original buffer for hash:" << hash;
    } else {
        int num = buffer.size() / 1000;
        float t_curValue = 0;
        for (int i = 0; i < buffer.size(); i += num) {
            if (i % num == 0) {
                t_buffer.append(buffer[i]);
            }
        }
        t_buffer.append(t_curValue);
        qCDebug(dmMusic) << "Downsampled buffer from" << buffer.size() << "to" << t_buffer.size() << "for hash:" << hash;
    }


    if (!t_buffer.isEmpty()) {
        qCDebug(dmMusic) << "Normalizing buffer data for hash:" << hash << "original size:" << t_buffer.size();
        auto max = *(std::max_element(std::begin(t_buffer), std::end(t_buffer)));
        for (int i = 0; i < t_buffer.size(); ++i) {
            float ft = t_buffer[i] / max;
            ft *= 1000;
            mappingbuf.append(ft);
            s_buffer.append(qAbs(t_buffer[i] / max));
        }
        qCDebug(dmMusic) << "Normalized buffer data for hash:" << hash;
    }

    if (!forceQuit) {
        QString userCachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QString path = userCachePath + "/wave/";

        QDir dir(path);
        if (!dir.exists()) {
            qCDebug(dmMusic) << "Creating wave cache directory:" << path;
            dir.mkdir(path);
        }
        path += QString("%1.dat").arg(hash);
        qCDebug(dmMusic) << "Writing wave cache to path:" << path;
        //write cache
        char *buf = new char[mappingbuf.size() * 4 ];
        memset(buf, 0, mappingbuf.size() * 4);
        FILE *fp = fopen(path.toUtf8().data(), "w+");
        if (fp != nullptr) {
            for (int i = 0; i < mappingbuf.size(); i++) {
                float ss = mappingbuf[i];
                memcpy(buf + i * 4, &ss, 4);
            }

            fwrite(buf, 4, mappingbuf.size(),  fp);
            qCInfo(dmMusic) << "Successfully saved audio wave cache for hash:" << hash << "to:" << path;
        } else {
            qCCritical(dmMusic) << "Failed to write audio wave cache file for hash:" << hash << "path:" << path;
            qWarning() << "can not write cache file " << hash << " failed";
        }
        if (fp)
            fclose(fp);
        delete []buf;
    } else {
        qCDebug(dmMusic) << "Force quit mode, skipping cache write for hash:" << hash;
    }
    Q_EMIT audioBufferFromThread(s_buffer, hash);
    qCDebug(dmMusic) << "Emitted audio buffer data for hash:" << hash << "size:" << s_buffer.size();
}

bool AudioDataDetector::queryCacheExisted(const QString &hash)
{
    qCDebug(dmMusic) << "Querying cache existence for hash:" << hash;
    QString path = DmGlobal::cachePath() + QString("/wave/%1.dat").arg(hash);
    if (!QFile::exists(path) && DmGlobal::playbackEngineType() != 1) {
        qCDebug(dmMusic) << "Cache file not found and engine type is not FFmpeg, using default data:" << path;
        path = ":/data/default_music.dat";
    }
    
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(dmMusic) << "Failed to open cache file for hash:" << hash << "path:" << path;
        return false;
    }
    if (file.size() == 0) {
        qCWarning(dmMusic) << "Cache file is empty for hash:" << hash << "path:" << path;
        return false;
    }

    qCDebug(dmMusic) << "Found valid cache file for hash:" << hash << "path:" << path << "size:" << file.size();

    QVector<float> f_buffer;

    //读取二进制数据
    while (!file.atEnd()) {
        float ss;
        file.read((char *)&ss, 4);
        f_buffer << qAbs(ss * 1.0 / 1000);
    }

    file.close();
    
    qCInfo(dmMusic) << "Successfully loaded audio buffer from cache for hash:" << hash << "buffer size:" << f_buffer.size();
    Q_EMIT audioBuffer(f_buffer, hash);
    return true;
}
