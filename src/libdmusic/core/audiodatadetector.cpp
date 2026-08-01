// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiodatadetector.h"

#include <QDir>
#include <QFile>
#include <QDebug>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

#include "dynamiclibraries.h"
#include "global.h"
#include "util/log.h"

// avformat
typedef AVFormatContext *(*avformat_alloc_context_function)(void);
typedef int (*avformat_open_input_function)(AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
typedef void (*avformat_free_context_function)(AVFormatContext *);
typedef int (*avformat_find_stream_info_function)(AVFormatContext *, AVDictionary **);
typedef int (*av_find_best_stream_function)(AVFormatContext *, enum AVMediaType, int, int, AVCodec **, int);
typedef void (*avformat_close_input_function)(AVFormatContext **);

// avcodec
typedef AVCodecContext *(*avcodec_alloc_context3_function)(const AVCodec *);
typedef int (*avcodec_parameters_to_context_function)(AVCodecContext *, const AVCodecParameters *);
typedef AVCodec *(*avcodec_find_decoder_function)(enum AVCodecID);
typedef int (*avcodec_open2_function)(AVCodecContext *, const AVCodec *, AVDictionary **);
typedef int (*avcodec_send_packet_function)(AVCodecContext *, const AVPacket *);
typedef int (*avcodec_receive_frame_function)(AVCodecContext *, AVFrame *);
typedef void (*avcodec_free_context_function)(AVCodecContext **);

// avutil
typedef AVPacket *(*av_packet_alloc_function)(void);
typedef AVFrame *(*av_frame_alloc_function)(void);
typedef int (*av_read_frame_function)(AVFormatContext *, AVPacket *);
typedef void (*av_packet_unref_function)(AVPacket *);
typedef void (*av_frame_free_function)(AVFrame **);
typedef void (*av_packet_free_function)(AVPacket **);
typedef int64_t (*av_rescale_rnd_function)(int64_t a, int64_t b, int64_t c, enum AVRounding);
typedef int (*av_samples_alloc_function)(uint8_t **, int *, int, int, enum AVSampleFormat, int);
typedef void (*av_freep_function)(void *);
typedef void (*av_channel_layout_default_function)(AVChannelLayout *, int);

// swresample
typedef int (*swr_alloc_set_opts2_function)(SwrContext **,
    const AVChannelLayout *, enum AVSampleFormat, int,
    const AVChannelLayout *, enum AVSampleFormat, int,
    int, void *);
typedef int (*swr_init_function)(SwrContext *);
typedef int (*swr_convert_function)(SwrContext *, uint8_t **, int, const uint8_t **, int);
typedef int64_t (*swr_get_delay_function)(SwrContext *, int64_t);
typedef void (*swr_free_function)(SwrContext **);

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

    auto fn_avformat_alloc_context = reinterpret_cast<avformat_alloc_context_function>(DynamicLibraries::instance()->resolve("avformat_alloc_context", true));
    auto fn_avformat_open_input = reinterpret_cast<avformat_open_input_function>(DynamicLibraries::instance()->resolve("avformat_open_input", true));
    auto fn_avformat_free_context = reinterpret_cast<avformat_free_context_function>(DynamicLibraries::instance()->resolve("avformat_free_context", true));
    auto fn_avformat_find_stream_info = reinterpret_cast<avformat_find_stream_info_function>(DynamicLibraries::instance()->resolve("avformat_find_stream_info", true));
    auto fn_av_find_best_stream = reinterpret_cast<av_find_best_stream_function>(DynamicLibraries::instance()->resolve("av_find_best_stream", true));
    auto fn_avformat_close_input = reinterpret_cast<avformat_close_input_function>(DynamicLibraries::instance()->resolve("avformat_close_input", true));
    auto fn_avcodec_alloc_context3 = reinterpret_cast<avcodec_alloc_context3_function>(DynamicLibraries::instance()->resolve("avcodec_alloc_context3", true));
    auto fn_avcodec_parameters_to_context = reinterpret_cast<avcodec_parameters_to_context_function>(DynamicLibraries::instance()->resolve("avcodec_parameters_to_context", true));
    auto fn_avcodec_find_decoder = reinterpret_cast<avcodec_find_decoder_function>(DynamicLibraries::instance()->resolve("avcodec_find_decoder", true));
    auto fn_avcodec_open2 = reinterpret_cast<avcodec_open2_function>(DynamicLibraries::instance()->resolve("avcodec_open2", true));
    auto fn_avcodec_send_packet = reinterpret_cast<avcodec_send_packet_function>(DynamicLibraries::instance()->resolve("avcodec_send_packet", true));
    auto fn_avcodec_receive_frame = reinterpret_cast<avcodec_receive_frame_function>(DynamicLibraries::instance()->resolve("avcodec_receive_frame", true));
    auto fn_avcodec_free_context = reinterpret_cast<avcodec_free_context_function>(DynamicLibraries::instance()->resolve("avcodec_free_context", true));
    auto fn_av_packet_alloc = reinterpret_cast<av_packet_alloc_function>(DynamicLibraries::instance()->resolve("av_packet_alloc", true));
    auto fn_av_frame_alloc = reinterpret_cast<av_frame_alloc_function>(DynamicLibraries::instance()->resolve("av_frame_alloc", true));
    auto fn_av_read_frame = reinterpret_cast<av_read_frame_function>(DynamicLibraries::instance()->resolve("av_read_frame", true));
    auto fn_av_packet_unref = reinterpret_cast<av_packet_unref_function>(DynamicLibraries::instance()->resolve("av_packet_unref", true));
    auto fn_av_frame_free = reinterpret_cast<av_frame_free_function>(DynamicLibraries::instance()->resolve("av_frame_free", true));
    auto fn_av_packet_free = reinterpret_cast<av_packet_free_function>(DynamicLibraries::instance()->resolve("av_packet_free", true));
    auto fn_av_rescale_rnd = reinterpret_cast<av_rescale_rnd_function>(DynamicLibraries::instance()->resolve("av_rescale_rnd", true));
    auto fn_av_samples_alloc = reinterpret_cast<av_samples_alloc_function>(DynamicLibraries::instance()->resolve("av_samples_alloc", true));
    auto fn_av_freep = reinterpret_cast<av_freep_function>(DynamicLibraries::instance()->resolve("av_freep", true));
    auto fn_av_channel_layout_default = reinterpret_cast<av_channel_layout_default_function>(DynamicLibraries::instance()->resolve("av_channel_layout_default", true));
    auto fn_swr_alloc_set_opts2 = reinterpret_cast<swr_alloc_set_opts2_function>(DynamicLibraries::instance()->resolve("swr_alloc_set_opts2", true));
    auto fn_swr_init = reinterpret_cast<swr_init_function>(DynamicLibraries::instance()->resolve("swr_init", true));
    auto fn_swr_convert = reinterpret_cast<swr_convert_function>(DynamicLibraries::instance()->resolve("swr_convert", true));
    auto fn_swr_get_delay = reinterpret_cast<swr_get_delay_function>(DynamicLibraries::instance()->resolve("swr_get_delay", true));
    auto fn_swr_free = reinterpret_cast<swr_free_function>(DynamicLibraries::instance()->resolve("swr_free", true));

    if (!fn_swr_alloc_set_opts2 || !fn_swr_init || !fn_swr_convert || !fn_av_rescale_rnd
        || !fn_av_samples_alloc || !fn_av_freep || !fn_av_channel_layout_default) {
        qCCritical(dmMusic) << "Failed to resolve swresample functions, cannot generate waveform";
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    AVFormatContext *pFormatCtx = fn_avformat_alloc_context();
    int ret = fn_avformat_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);
    if (pFormatCtx == nullptr || ret != 0) {
        qCCritical(dmMusic) << "Failed to open input format context for file:" << path << "error code:" << ret;
        fn_avformat_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    qCDebug(dmMusic) << "Successfully opened format context for file:" << path;

    ret = fn_avformat_find_stream_info(pFormatCtx, nullptr);
    if (ret < 0) {
        qCWarning(dmMusic) << "Failed to find stream info for file:" << path << "error code:" << ret;
    }

    int audioIdx = fn_av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audioIdx < 0) {
        qCWarning(dmMusic) << "No audio stream found in file:" << path;
        fn_avformat_close_input(&pFormatCtx);
        fn_avformat_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    qCDebug(dmMusic) << "Found audio stream at index:" << audioIdx << "for file:" << path;

    AVCodecParameters *codecpar = pFormatCtx->streams[audioIdx]->codecpar;
    AVCodecContext *codecCtx = fn_avcodec_alloc_context3(nullptr);
    fn_avcodec_parameters_to_context(codecCtx, codecpar);

    const AVCodec *codec = fn_avcodec_find_decoder(codecCtx->codec_id);
    if (!codec) {
        qCCritical(dmMusic) << "Failed to find decoder for codec ID:" << codecCtx->codec_id << "in file:" << path;
        fn_avcodec_free_context(&codecCtx);
        fn_avformat_close_input(&pFormatCtx);
        fn_avformat_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    if (fn_avcodec_open2(codecCtx, codec, nullptr) < 0) {
        qCCritical(dmMusic) << "Failed to open codec for file:" << path;
        fn_avcodec_free_context(&codecCtx);
        fn_avformat_close_input(&pFormatCtx);
        fn_avformat_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    qCDebug(dmMusic) << "Successfully initialized codec for file:" << path;

    AVChannelLayout outLayout;
    fn_av_channel_layout_default(&outLayout, 1);

    SwrContext *swr = nullptr;
    if (fn_swr_alloc_set_opts2(&swr,
            &outLayout, AV_SAMPLE_FMT_FLT, 44100,
            &codecCtx->ch_layout, codecCtx->sample_fmt, codecCtx->sample_rate,
            0, nullptr) < 0
        || fn_swr_init(swr) < 0) {
        qCCritical(dmMusic) << "Failed to init SwrContext for:" << path;
        if (swr) fn_swr_free(&swr);
        fn_avcodec_free_context(&codecCtx);
        fn_avformat_close_input(&pFormatCtx);
        fn_avformat_free_context(pFormatCtx);
        m_curPath.clear();
        m_curHash.clear();
        return;
    }

    AVPacket *packet = fn_av_packet_alloc();
    AVFrame *frame = fn_av_frame_alloc();

    QVector<float> curData;
    const int grainSize = 256;
    float currentPeak = 0;
    int count = 0;

    while (fn_av_read_frame(pFormatCtx, packet) >= 0) {
        if (m_stopFlag && curData.size() > 10) {
            qCDebug(dmMusic) << "Stop flag detected, cleaning up resources for file:" << path;
            fn_av_packet_unref(packet);
            fn_av_packet_free(&packet);
            fn_av_frame_free(&frame);
            fn_swr_free(&swr);
            fn_avcodec_free_context(&codecCtx);
            fn_avformat_close_input(&pFormatCtx);
            fn_avformat_free_context(pFormatCtx);
            if (currentPeak > 0) curData.append(currentPeak);
            resample(curData, hash, true);
            m_stopFlag = false;
            m_curPath.clear();
            m_curHash.clear();
            qCDebug(dmMusic) << "Successfully stopped detection for file:" << path;
            return;
        }

        if (packet->stream_index != audioIdx) {
            fn_av_packet_unref(packet);
            continue;
        }

        if (fn_avcodec_send_packet(codecCtx, packet) != 0) {
            fn_av_packet_unref(packet);
            continue;
        }

        while (fn_avcodec_receive_frame(codecCtx, frame) == 0) {
            int out_samples = fn_av_rescale_rnd(
                fn_swr_get_delay(swr, frame->sample_rate) + frame->nb_samples,
                44100, frame->sample_rate, AV_ROUND_UP);

            uint8_t *out_data = nullptr;
            fn_av_samples_alloc(&out_data, nullptr, 1, out_samples, AV_SAMPLE_FMT_FLT, 0);

            int converted = fn_swr_convert(swr, &out_data, out_samples,
                (const uint8_t **)frame->data, frame->nb_samples);

            float *floatData = reinterpret_cast<float *>(out_data);
            for (int i = 0; i < converted; ++i) {
                float absSample = qAbs(floatData[i]);
                if (absSample > currentPeak) currentPeak = absSample;
                if (++count >= grainSize) {
                    curData.append(currentPeak);
                    currentPeak = 0;
                    count = 0;
                }
            }
            fn_av_freep(&out_data);
        }
        fn_av_packet_unref(packet);
    }

    if (currentPeak > 0) curData.append(currentPeak);

    fn_av_packet_free(&packet);
    fn_av_frame_free(&frame);
    fn_swr_free(&swr);
    fn_avcodec_free_context(&codecCtx);
    fn_avformat_close_input(&pFormatCtx);
    fn_avformat_free_context(pFormatCtx);

    qCInfo(dmMusic) << "Waveform detection completed for:" << path << "grains:" << curData.size();
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
        for (int i = 0; i < buffer.size(); i += num) {
            t_buffer.append(buffer[i]);
        }
        qCDebug(dmMusic) << "Downsampled buffer from" << buffer.size() << "to" << t_buffer.size() << "for hash:" << hash;
    }

    if (!t_buffer.isEmpty()) {
        qCDebug(dmMusic) << "Normalizing buffer data for hash:" << hash << "original size:" << t_buffer.size();
        float maxAbs = 0;
        for (int i = 0; i < t_buffer.size(); ++i) {
            float absVal = qAbs(t_buffer[i]);
            if (absVal > maxAbs) maxAbs = absVal;
        }
        if (maxAbs > 0) {
            for (int i = 0; i < t_buffer.size(); ++i) {
                float normalizedValue = qAbs(t_buffer[i]) / maxAbs;
                mappingbuf.append(normalizedValue * 1000);
                s_buffer.append(normalizedValue);
            }
        } else {
            mappingbuf.fill(0, t_buffer.size());
            s_buffer.fill(0, t_buffer.size());
        }
        qCDebug(dmMusic) << "Normalized buffer data for hash:" << hash;
    }

    if (!forceQuit) {
        QString dirPath = DmGlobal::cachePath() + "/wave/";
        QDir dir(dirPath);
        if (!dir.exists()) {
            qCDebug(dmMusic) << "Creating wave cache directory:" << dirPath;
            dir.mkdir(dirPath);
        }
        QString filePath = dirPath + QString("%1.dat").arg(hash);
        qCDebug(dmMusic) << "Writing wave cache to path:" << filePath;
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            for (int i = 0; i < mappingbuf.size(); i++) {
                float ss = mappingbuf[i];
                file.write((const char *)&ss, 4);
            }
            file.close();
            qCInfo(dmMusic) << "Successfully saved audio wave cache for hash:" << hash << "to:" << filePath;
        } else {
            qCCritical(dmMusic) << "Failed to write audio wave cache file for hash:" << hash << "path:" << filePath;
            qWarning() << "can not write cache file " << hash << " failed";
        }
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
    while (!file.atEnd()) {
        float ss;
        qint64 bytesRead = file.read((char *)&ss, 4);
        if (bytesRead == 4) {
            f_buffer << ss / 1000;
        } else {
            qCWarning(dmMusic) << "Incomplete float read from wave cache, expected 4 bytes, got" << bytesRead << "for hash:" << hash;
            break;
        }
    }
    file.close();

    qCInfo(dmMusic) << "Successfully loaded audio buffer from cache for hash:" << hash << "buffer size:" << f_buffer.size();
    Q_EMIT audioBuffer(f_buffer, hash);
    return true;
}
