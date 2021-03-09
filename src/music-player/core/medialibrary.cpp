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

#include "medialibrary.h"

#include <QDebug>
#include <QFileInfo>
#include <QDirIterator>

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

#include <mediameta.h>
#include <metadetector.h>

#ifdef SUPPORT_INOTIFY
#include "util/inotifyengine.h"
#endif

#include "player.h"
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


MediaLibrary::MediaLibrary(QObject *parent) : QObject(parent)
{
}


MediaLibrary::~MediaLibrary()
{

}

MediaMeta MediaLibrary::creatMediaMeta(QString path)
{
    MediaMeta mediaMeta;
    QFileInfo fileinfo(path);
    while (fileinfo.isSymLink()) {  //to find final target
        fileinfo.setFile(fileinfo.symLinkTarget());
    }
    auto hash = DMusic::filepathHash(fileinfo.absoluteFilePath());
    mediaMeta.hash = hash;
    mediaMeta = MetaDetector::getInstance()->updateMetaFromLocalfile(mediaMeta, fileinfo);

    return mediaMeta;
}

QStringList MediaLibrary::getSupportedSuffixs()
{
    return  Player::getInstance()->supportedSuffixList();
}

void MediaLibrary::init()
{
    MetaDetector::getInstance();
}
