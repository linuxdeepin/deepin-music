// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "apeseekfile.h"

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QScopeGuard>
#include <QThread>
#include <QtEndian>

#include <limits>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

static bool makeRoomForApeSeekFile(const QFileInfo &destination, qint64 incomingSize)
{
    // A RIFF/WAV file can fit in this budget even at its maximum supported size.
    constexpr qint64 cacheLimit = 4LL * 1024 * 1024 * 1024;
    static const QRegularExpression cacheName(QStringLiteral("^[0-9a-f]{32}-[0-9]+--?[0-9]+\\.wav$"));
    const auto files = destination.dir().entryInfoList(
        {QStringLiteral("*.wav")}, QDir::Files | QDir::NoSymLinks, QDir::Time | QDir::Reversed);
    QFileInfoList cachedFiles;
    qint64 totalSize = incomingSize;
    for (const auto &file : files) {
        if (file.fileName() != destination.fileName() && cacheName.match(file.fileName()).hasMatch()) {
            totalSize += file.size();
            cachedFiles.append(file);
        }
    }
    for (const auto &file : cachedFiles) {
        if (totalSize <= cacheLimit) {
            break;
        }
        if (QFile::remove(file.absoluteFilePath())) {
            totalSize -= file.size();
        }
    }
    return totalSize <= cacheLimit;
}

bool writeApeSeekFile(const QFileInfo &source, const QString &destination)
{
    const auto sourceMatches = [&source] {
        const QFileInfo current(source.absoluteFilePath());
        return current.isFile() && current.size() == source.size()
               && current.lastModified() == source.lastModified();
    };
    if (!sourceMatches()) {
        return false;
    }

    AVFormatContext *input = nullptr;
    const auto closeInput = qScopeGuard([&] { avformat_close_input(&input); });
#ifdef Q_OS_WIN
    const QByteArray path = "file:" + source.absoluteFilePath().toUtf8();
#else
    const QByteArray path = "file:" + QFile::encodeName(source.absoluteFilePath());
#endif
    const AVInputFormat *apeFormat = av_find_input_format("ape");
    if (!apeFormat || avformat_open_input(&input, path.constData(), apeFormat, nullptr) < 0
        || avformat_find_stream_info(input, nullptr) < 0) {
        return false;
    }
    const AVCodec *decoder = nullptr;
    const int stream = av_find_best_stream(input, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
    if (stream < 0 || !decoder || decoder->id != AV_CODEC_ID_APE) {
        return false;
    }

    AVCodecContext *codec = avcodec_alloc_context3(decoder);
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    const auto freeDecoder = qScopeGuard([&] {
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&codec);
    });
    if (!codec || !packet || !frame
        || avcodec_parameters_to_context(codec, input->streams[stream]->codecpar) < 0
        || avcodec_open2(codec, decoder, nullptr) < 0) {
        return false;
    }

    // APE's planar integer PCM can be interleaved without resampling or reducing precision.
    const int channels = codec->ch_layout.nb_channels;
    const int bytesPerSample = av_get_bytes_per_sample(codec->sample_fmt);
    if (channels < 1 || channels > 2 || codec->sample_rate <= 0
        || (codec->sample_fmt != AV_SAMPLE_FMT_U8P && codec->sample_fmt != AV_SAMPLE_FMT_S16P
            && codec->sample_fmt != AV_SAMPLE_FMT_S32P)) {
        return false;
    }
    const int blockAlign = channels * bytesPerSample;
    constexpr qint64 headerSize = 44;
    constexpr qint64 maxDataSize = std::numeric_limits<quint32>::max() - 37;
    QSaveFile output(destination);
    if (!output.open(QIODevice::WriteOnly) || !output.seek(headerSize)) {
        return false;
    }

    const auto interrupted = [] { return QThread::currentThread()->isInterruptionRequested(); };
    QByteArray pcm;
    qint64 dataSize = 0;
    const auto decodePacket = [&](const AVPacket *packetToDecode) {
        if (avcodec_send_packet(codec, packetToDecode) < 0) {
            return false;
        }
        while (!interrupted()) {
            const int result = avcodec_receive_frame(codec, frame);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                return true;
            }
            const qint64 frameBytes = qint64(frame->nb_samples) * blockAlign;
            if (result < 0 || frameBytes <= 0 || frameBytes > maxDataSize - dataSize
                || frame->format != codec->sample_fmt || frame->ch_layout.nb_channels != channels) {
                return false;
            }
            pcm.resize(frameBytes);
            auto *dest = reinterpret_cast<uchar *>(pcm.data());
            for (int sample = 0; sample < frame->nb_samples; ++sample) {
                for (int channel = 0; channel < channels; ++channel) {
                    const auto *src = frame->extended_data[channel] + sample * bytesPerSample;
                    if (bytesPerSample == 1) {
                        *dest = *src;
                    } else if (bytesPerSample == 2) {
                        qToLittleEndian(qFromUnaligned<quint16>(src), dest);
                    } else {
                        qToLittleEndian(qFromUnaligned<quint32>(src), dest);
                    }
                    dest += bytesPerSample;
                }
            }
            if (output.write(pcm) != frameBytes) {
                return false;
            }
            dataSize += frameBytes;
        }
        return false;
    };

    int readResult = 0;
    while (!interrupted() && (readResult = av_read_frame(input, packet)) >= 0) {
        if (packet->stream_index == stream && !decodePacket(packet)) {
            return false;
        }
        av_packet_unref(packet);
    }
    if (readResult != AVERROR_EOF || !decodePacket(nullptr) || dataSize == 0) {
        return false;
    }

    const int padding = dataSize % 2;
    if ((padding && !output.putChar('\0')) || !output.seek(0)) {
        return false;
    }
    QDataStream header(&output);
    header.setByteOrder(QDataStream::LittleEndian);
    header.writeRawData("RIFF", 4);
    header << quint32(dataSize + padding + 36);
    header.writeRawData("WAVEfmt ", 8);
    header << quint32(16) << quint16(1) << quint16(channels) << quint32(codec->sample_rate)
           << quint32(codec->sample_rate * blockAlign) << quint16(blockAlign) << quint16(bytesPerSample * 8);
    header.writeRawData("data", 4);
    header << quint32(dataSize);
    if (header.status() != QDataStream::Ok || interrupted() || !sourceMatches()) {
        return false;
    }
    const QFileInfo cacheFile(destination);
    QLockFile cacheLock(cacheFile.dir().absoluteFilePath(QStringLiteral(".seekfile.lock")));
    if (!cacheLock.tryLock(1000) || interrupted() || !sourceMatches()) {
        return false;
    }
    return makeRoomForApeSeekFile(cacheFile, headerSize + dataSize + padding) && output.commit();
}

bool isApeSeekFileReady(const QString &path)
{
    QFile file(path);
    if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const auto header = file.read(44);
    if (header.size() != 44 || !header.startsWith("RIFF") || header.mid(8, 8) != "WAVEfmt "
        || header.mid(36, 4) != "data") {
        return false;
    }
    const qint64 dataSize = qFromLittleEndian<quint32>(header.constData() + 40);
    return dataSize > 0 && file.size() == 44 + dataSize + dataSize % 2;
}
