// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audioanalysis.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif // __cplusplus

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/apetag.h>
#include <taglib/synchronizedlyricsframe.h>
#include <taglib/unsynchronizedlyricsframe.h>

#include <QAudioInput>
#include <QTextCodec>
#include <QFileInfo>
#include <QDateTime>
#include <QTextCodec>
#include <QDir>
#include <QImage>
#include <QBuffer>
#include <QDebug>
#include <QImageReader>
#include <QAudioFormat>
#include <QAudioSource>
#include <QMediaDevices>

#include "dynamiclibraries.h"
#include "global.h"
#include "utils.h"
#include "audiodatadetector.h"

#if QT_VERSION >= 0x040000
# define QStringToTString(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8)
#else
# define QStringToTString(s) TagLib::String(s.utf8().data(), TagLib::String::UTF8)
#endif

static QMap<QString, QByteArray> localeCodes;
static qint64 preAddTime = -1;

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

class AudioAnalysisPrivate
{
public:
    AudioAnalysisPrivate(AudioAnalysis *parent)
        : m_parent(parent)
    {
        localeCodes.insert("zh_CN", "GB18030");
    }

private:
    friend class AudioAnalysis;
    AudioAnalysis                  *m_parent                  = nullptr;
    QAudioSource                   *m_audioSource             = nullptr;
    QIODevice                      *m_audioDevice             = nullptr;
    AudioDataDetector              *m_audioDataDetector       = nullptr;
};

AudioAnalysis::AudioAnalysis(QObject *parent)
    : QObject(parent), m_data(new AudioAnalysisPrivate(this))
{
    m_data->m_audioDataDetector = new AudioDataDetector(this);
    connect(m_data->m_audioDataDetector, &AudioDataDetector::audioBuffer, this, &AudioAnalysis::audioBuffer);
}

void AudioAnalysis::parseAudioBuffer(const DMusic::MediaMeta &meta)
{
    m_data->m_audioDataDetector->onClearBufferDetector();
    m_data->m_audioDataDetector->onBufferDetector(meta.localPath, meta.hash);
}

DMusic::MediaMeta AudioAnalysis::creatMediaMeta(const QString &path)
{
    DMusic::MediaMeta mediaMeta;
    QFileInfo fileinfo(path);
    while (fileinfo.isSymLink()) {  //to find final target
        fileinfo.setFile(fileinfo.symLinkTarget());
    }
    auto hash = Utils::filePathHash(fileinfo.absoluteFilePath());
    mediaMeta.hash = hash;
    mediaMeta.localPath = path;
    parseMetaFromLocalFile(mediaMeta);

    return mediaMeta;
}

void AudioAnalysis::convertMetaCodec(DMusic::MediaMeta &meta, const QString &codecName)
{
    QTextCodec *codec = QTextCodec::codecForName(codecName.toLatin1());
    if (codec != nullptr) {
        meta.album = codec->toUnicode(meta.originalAlbum);
        meta.artist = codec->toUnicode(meta.originalArtist);
        meta.title = codec->toUnicode(meta.originalTitle);
    }
    if (meta.title.isEmpty()) {
        QFileInfo localFi(meta.localPath);
        meta.title = localFi.completeBaseName();
    }
    if (meta.album.isEmpty()) {
        meta.album = DmGlobal::unknownAlbumText();
    }
    if (meta.artist.isEmpty()) {
        meta.artist = DmGlobal::unknownArtistText();
    }
    meta.codec = codecName;
    Utils::updateChineseMetaInfo(meta);
}

bool AudioAnalysis::parseFileTagCodec(DMusic::MediaMeta &meta)
{
    if (meta.localPath.isEmpty()) {
        qCritical() << "meta localPath is empty:" << meta.title << meta.hash;
        return false;
    }

    QByteArray detectByte;
    QString detectCodec;
    auto mediaPath = QStringToTString(meta.localPath);
#ifdef _WIN32
    TagLib::FileRef f(meta->localPath.toStdWString().c_str());
#else
    TagLib::FileRef f(meta.localPath.toStdString().c_str());
#endif
    TagLib::Tag *tag = f.tag();

    if (!f.file() || !tag) {
        qCritical() << "TagLib: open file failed:" << meta.localPath << f.file();
        meta.localPath.clear();
        return false;
    }

    TagLib::AudioProperties *t_audioProperties = f.audioProperties();
    if (t_audioProperties == nullptr) {
        meta.localPath.clear();
        return false;
    }
    meta.length = t_audioProperties->length() * 1000;

    bool encode = true;
    encode &= tag->title().isNull() ? true : tag->title().isLatin1();
    encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
    encode &= tag->album().isNull() ? true : tag->album().isLatin1();
    if (encode) {
        if (detectCodec.isEmpty()) {
            detectByte += tag->title().toCString();
            detectByte += tag->artist().toCString();
            detectByte += tag->album().toCString();
            auto allDetectCodecs = Utils::detectEncodings(detectByte);
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
                if (Utils::isChinese(ch)) {
                    detectCodec = "GB18030";
                    break;
                }
            }
        }

        QString detectCodecStr(detectCodec);
        if (detectCodecStr.compare("utf-8", Qt::CaseInsensitive) == 0) {
            meta.album = TStringToQString(tag->album());
            meta.artist = TStringToQString(tag->artist());
            meta.title = TStringToQString(tag->title());
            meta.codec = "UTF-8";  //info codec
        } else {
            QTextCodec *codec = QTextCodec::codecForName(detectCodec.toLatin1());
            if (codec == nullptr) {
                meta.album = TStringToQString(tag->album());
                meta.artist = TStringToQString(tag->artist());
                meta.title = TStringToQString(tag->title());
                meta.codec = "UTF-8";
            } else {
                meta.album = codec->toUnicode(tag->album().toCString());
                meta.artist = codec->toUnicode(tag->artist().toCString());
                meta.title = codec->toUnicode(tag->title().toCString());
            }
            meta.codec = detectCodec;
        }
    } else {
        meta.album = TStringToQString(tag->album());
        meta.artist = TStringToQString(tag->artist());
        meta.title = TStringToQString(tag->title());
        meta.codec = "UTF-8";
    }

    if (meta.title.isEmpty()) {
        QFileInfo localFi(meta.localPath);
        meta.title = localFi.completeBaseName();
    }

    auto ss = tag->album();
    meta.originalAlbum = QByteArray(tag->album().toCString(true));
    meta.originalArtist = QByteArray(tag->artist().toCString(true));
    meta.originalTitle = QByteArray(tag->title().toCString(true));

    //empty str
    meta.album = meta.album.simplified();
    meta.artist = meta.artist.simplified();
    meta.title = meta.title.simplified();
    return  true;
}

bool AudioAnalysis::parseMetaFromLocalFile(DMusic::MediaMeta &meta)
{
    if (meta.localPath.isEmpty()) {
        return false;
    }
    QString curFilePath = meta.localPath;
    QFileInfo fileInfo(curFilePath);
    meta.length = 0;

    parseFileTagCodec(meta);

    if (meta.length == 0 && DmGlobal::playbackEngineType() == 1) {
        //#ifndef DISABLE_LIBAV
        format_alloc_context_function format_alloc_context = (format_alloc_context_function)DynamicLibraries::instance()->resolve("avformat_alloc_context", true);
        format_open_input_function format_open_input = (format_open_input_function)DynamicLibraries::instance()->resolve("avformat_open_input", true);
        format_find_stream_info_function format_find_stream_info = (format_find_stream_info_function)DynamicLibraries::instance()->resolve("avformat_find_stream_info", true);
        format_close_input_function format_close_input = (format_close_input_function)DynamicLibraries::instance()->resolve("avformat_close_input", true);
        format_free_context_function format_free_context = (format_free_context_function)DynamicLibraries::instance()->resolve("avformat_free_context", true);
        AVFormatContext *pFormatCtx = format_alloc_context();
        format_open_input(&pFormatCtx, curFilePath.toStdString().c_str(), nullptr, nullptr);
        if (pFormatCtx) {
            int ret = format_find_stream_info(pFormatCtx, nullptr);
            if (ret < 0)
                return false;
            bool hasAudio = false;
            for (int i = 0; i < pFormatCtx->nb_streams; i++)
                if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                    hasAudio = true;
            if (!hasAudio)
                return false;
            int64_t duration = pFormatCtx->duration / 1000;
            if (duration > 0) {
                meta.length = duration;
                meta.localPath = curFilePath;
            }
        }
        format_close_input(&pFormatCtx);
        format_free_context(pFormatCtx);
    }

    meta.size = fileInfo.size();

    auto current = QDateTime::currentDateTime();
    // HACK how to sort by add time
    meta.timestamp = current.toMSecsSinceEpoch() * 1000;
    // 同时添加
    if (preAddTime >= meta.timestamp) {
        meta.timestamp += 1;
        preAddTime = meta.timestamp;
    }
    meta.filetype = fileInfo.suffix().toLower(); //歌曲文件后缀使用小写保存

    if (meta.title.isEmpty()) {
        meta.title = fileInfo.completeBaseName();
    }
    if (meta.album.isEmpty()) {
        meta.album = DmGlobal::unknownAlbumText();
    }
    if (meta.artist.isEmpty()) {
        meta.artist = DmGlobal::unknownArtistText();
    }
    Utils::updateChineseMetaInfo(meta);

    return true;
}

QStringList AudioAnalysis::detectEncodings(const DMusic::MediaMeta &meta)
{
    if (meta.localPath.isEmpty()) {
        return QStringList() << "UTF-8";
    }
    QByteArray detectByte;

    if (!meta.cuePath.isEmpty()) {
        QFile cueFile(meta.cuePath);
        if (cueFile.open(QIODevice::ReadOnly)) {
            detectByte =  cueFile.readAll();
            return Utils::detectEncodings(detectByte);
        }
    }

    detectByte += meta.originalAlbum;
    detectByte += meta.originalTitle;
    detectByte += meta.originalArtist;

    return Utils::detectEncodings(detectByte);
}

void AudioAnalysis::parseMetaCover(DMusic::MediaMeta &meta)
{
    int engineType = DmGlobal::playbackEngineType();
    QString tmpPath = DmGlobal::cachePath();
    QString hash = meta.hash;
    QString path = meta.localPath;
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
                meta.coverUrl = imagesDirPath + "/" + imageName;
                meta.hasimage = true;
                return;
            }
        }
    }

//#ifndef DISABLE_LIBAV
    if (!path.isEmpty()) {
        QImage image;
        if (engineType == 1) {
            format_alloc_context_function format_alloc_context = (format_alloc_context_function)DynamicLibraries::instance()->resolve("avformat_alloc_context", true);
            format_open_input_function format_open_input = (format_open_input_function)DynamicLibraries::instance()->resolve("avformat_open_input", true);
            format_close_input_function format_close_input = (format_close_input_function)DynamicLibraries::instance()->resolve("avformat_close_input", true);
            format_free_context_function format_free_context = (format_free_context_function)DynamicLibraries::instance()->resolve("avformat_free_context", true);

            AVFormatContext *pFormatCtx = format_alloc_context();
            format_open_input(&pFormatCtx, path.toStdString().c_str(), nullptr, nullptr);

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
        }
        
        // ffmpeg 没有解析出来 尝试直接读取ID3v2
        if (image.isNull()) {
#ifdef _WIN32
            TagLib::MPEG::File f(path.toStdString().c_str());
#else
            TagLib::MPEG::File f(path.toStdString().c_str());
#endif
            // 检查音乐文件
            if (f.isValid()) {
                // 音乐文件不一定存在ID3v2Tag
                if (f.ID3v2Tag() != nullptr) {
                    TagLib::ID3v2::FrameList frameList = f.ID3v2Tag()->frameListMap()["APIC"];
                    if (!frameList.isEmpty()) {
                        TagLib::ID3v2::AttachedPictureFrame *picFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front());
                        QBuffer buffer;
                        buffer.setData(picFrame->picture().data(), static_cast<int>(picFrame->picture().size()));
                        QImageReader imageReader(&buffer);
                        image = imageReader.read();
                    }
                }

                f.clear();
            }
        }

        if (!image.isNull()) {
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "jpg");
            image = image.scaled(QSize(200, 200), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            image.save(imagesDirPath + "/" + imageName);
            meta.coverUrl = imagesDirPath + "/" + imageName;
            meta.hasimage = true;
        }
    }
}

QImage AudioAnalysis::getMetaCoverImage(DMusic::MediaMeta meta)
{
    QImage image;
    if (meta.hasimage) {
        int engineType = DmGlobal::playbackEngineType();
        if (engineType == 1) {
            format_alloc_context_function format_alloc_context = (format_alloc_context_function)DynamicLibraries::instance()->resolve("avformat_alloc_context", true);
            format_open_input_function format_open_input = (format_open_input_function)DynamicLibraries::instance()->resolve("avformat_open_input", true);
            format_close_input_function format_close_input = (format_close_input_function)DynamicLibraries::instance()->resolve("avformat_close_input", true);
            format_free_context_function format_free_context = (format_free_context_function)DynamicLibraries::instance()->resolve("avformat_free_context", true);

            AVFormatContext *pFormatCtx = format_alloc_context();
            format_open_input(&pFormatCtx, meta.localPath.toUtf8().data(), nullptr, nullptr);

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
        }
        
        // ffmpeg 没有解析出来 尝试直接读取ID3v2
        if (image.isNull()) {
#ifdef _WIN32
            TagLib::MPEG::File f(meta.localPath.toStdWString().c_str());
#else
            TagLib::MPEG::File f(meta.localPath.toStdString().c_str());
#endif
            // 检查音乐文件
            if (f.isValid()) {
                // 音乐文件不一定存在ID3v2Tag
                if (f.ID3v2Tag()) {
                    TagLib::ID3v2::FrameList frameList = f.ID3v2Tag()->frameListMap()["APIC"];
                    if (!frameList.isEmpty()) {
                        TagLib::ID3v2::AttachedPictureFrame *picFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front());
                        QBuffer buffer;
                        buffer.setData(picFrame->picture().data(), static_cast<int>(picFrame->picture().size()));
                        QImageReader imageReader(&buffer);
                        image = imageReader.read();
                    }
                }

                f.clear();
            }
        }
    }

    // 默认图片
    if (image.isNull()) {
        image = QImage(DmGlobal::cachePath() + "/images/default_cover.png");
    }

    return image;
}

void AudioAnalysis::parseMetaLyrics(DMusic::MediaMeta &meta)
{
    QString tmpPath = DmGlobal::cachePath();
    QString hash = meta.hash;
    QString path = meta.localPath;
    QString lyricDirPath = tmpPath + QDir::separator() + "lyrics";
    QString lyricName = hash + ".lrc";
    QDir lyricDir(lyricDirPath);
    if (!lyricDir.exists()) {
        lyricDir.cdUp();
        lyricDir.mkdir("lyrics");
        lyricDir.cd("lyrics");
    }

    if (!tmpPath.isEmpty() && !hash.isEmpty()) {
        // 歌词文件存在，停止解析
        if (lyricDir.exists(lyricName)) {
            return;
        }

        if (!path.isEmpty()) {
            QFile lyric(lyricDirPath + QDir::separator() + lyricName);
            TagLib::MPEG::File f(path.toStdString().c_str());
            QString lyricStr = "";

            // 检查音乐文件
            if (f.isValid()) {
                // 音乐文件不一定存在ID3v2Tag
                if (f.ID3v2Tag() != nullptr) {
                    // 先获取同步歌词
                    TagLib::ID3v2::FrameList syltFrames = f.ID3v2Tag()->frameListMap()["SYLT"];
                    if (!syltFrames.isEmpty()) {
                        TagLib::ID3v2::SynchronizedLyricsFrame *frame = dynamic_cast<TagLib::ID3v2::SynchronizedLyricsFrame *>(syltFrames.front());
                        if (frame) {
                            TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList synchedTextList = frame->synchedText();
                            for (unsigned int i = 0; i < synchedTextList.size(); i++){
                                QString time = QDateTime::fromMSecsSinceEpoch(synchedTextList[i].time).toString("mm:ss.zzz");
                                QString text = TStringToQString(synchedTextList[i].text).trimmed();
                                lyricStr.append(QString("[%1]%2\n").arg(time).arg(text));
                            }
                        }
                    }

                    // 没获取到歌词，获取非同步歌词
                    if (lyricStr.isEmpty()) {
                        TagLib::ID3v2::FrameList usltFrames = f.ID3v2Tag()->frameListMap()["USLT"];
                        if (!usltFrames.isEmpty()) {
                            TagLib::ID3v2::UnsynchronizedLyricsFrame *frame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(usltFrames.front());
                            if (frame) {
                                lyricStr = TStringToQString(frame->text());
                            }
                        }
                    }

                    if (!lyricStr.isEmpty()) {
                        if (lyric.open(QIODevice::WriteOnly)) {
                            lyric.write(lyricStr.toUtf8());
                        }
                        lyric.close();
                    }
                }

                f.clear();
            }
        }
    }
}

void AudioAnalysis::startRecorder()
{
    // 初始化
    if (m_data->m_audioDevice == nullptr) {
        QAudioFormat audioFormat;
        //TODO: 设置小端输出及格式
        // audioFormat.setByteOrder(QAudioFormat::LittleEndian);
        // audioFormat.setCodec("audio/pcm");
        audioFormat.setChannelCount(1);
        audioFormat.setSampleRate(44100);
        audioFormat.setSampleFormat(QAudioFormat::Int16);

        QAudioDevice devInfo = QMediaDevices::defaultAudioOutput();
        if (devInfo.isNull()) qDebug() << __func__;
        if (!devInfo.isFormatSupported(audioFormat)) qDebug() << __func__;

        if (nullptr == m_data->m_audioSource)
            m_data->m_audioSource = new QAudioSource(devInfo, audioFormat, this);
        // TODO:source连接问题待确认
        // m_data->m_audioDevice = m_data->m_audioSource->start();
        connect(m_data->m_audioDevice, &QIODevice::readyRead, this, &AudioAnalysis::parseData);
    } else {
        m_data->m_audioSource->resume();
    }
}

void AudioAnalysis::suspendRecorder()
{
    if (nullptr != m_data->m_audioSource) {
        m_data->m_audioSource->suspend();
    }
}

void AudioAnalysis::stopRecorder()
{
    if (nullptr != m_data->m_audioSource) {
        m_data->m_audioSource->stop();
    }
    m_data->m_audioDevice->deleteLater();
    m_data->m_audioDevice = nullptr;
}

void AudioAnalysis::parseData()
{
    QByteArray datas = m_data->m_audioDevice->readAll();
    qint16 *pdata = (qint16 *)(datas.data());

    std::complex<float> sampleData[1024];
    for (int i = 0; i < 1024; ++i) {
        sampleData[i] = (i * 2) >= datas.size() ? std::complex<float>(0, 0) : std::complex<float>(pdata[i], 0);
    }
    int log2N = (int)log2(1024.0 - 1.0) + 1;
    Utils::fft(sampleData, log2N, -1);
    QVector<int> curSampleData;
    for (int i = 0; i < 1024; ++i) {
        curSampleData.append(abs(sampleData[i]) / sqrt(2) / 2);
    }

    emit audioSpectrumData(curSampleData);
}
