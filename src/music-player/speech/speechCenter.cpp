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

#include "speechCenter.h"

#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>

#include "musicsettings.h"
#include "commonservice.h"
#include "databaseservice.h"
#include "player.h"

SpeechCenter::SpeechCenter(QObject *parent)
    : QObject(parent),
      m_needRefresh(false)
{
    m_settings = Dtk::Core::DSettings::fromJsonFile(":/data/deepin-music-speechreply.json");
}

//指定歌曲
QVariant SpeechCenter::playMusic(QString musicName)
{
    // 去掉空格
    musicName = musicName.simplified();
    qDebug() << __FUNCTION__ << musicName;
    //跳转到所有音乐
    m_MediaMetas.clear();
    m_needRefresh = true;
    QString str;
    if (musicName.isEmpty()) {
        m_MediaMetas = DataBaseService::getInstance()->allMusicInfos();
        if (m_MediaMetas.size() > 0) {
            emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "all");
            QTime time;
            int index = 0;
            time = QTime::currentTime();
            //qsrand(static_cast<uint>((time.msec() + time.second() * 1000)));
            //index = qrand() % m_MediaMetas.size();
            index = static_cast<int>(QRandomGenerator::global()->bounded((time.msec() + time.second() * 1000)) % m_MediaMetas.size());
            MediaMeta mediaMeta = m_MediaMetas.at(index);
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(m_MediaMetas);
            //设置当前播放为查询出的音乐
            Player::getInstance()->setCurrentPlayListHash("all", false);
            Player::getInstance()->playMeta(mediaMeta);
            emit CommonService::getInstance()->sigScrollToCurrentPosition("all");
            // 通知播放队列刷新
            emit Player::getInstance()->signalPlayListChanged();
            str = m_settings->value("speechreply.speech.ok").toString();
        } else {
            // 没有歌曲
            str = m_settings->value("speechreply.speech.playMusicIsEmptyError").toString();
        }
    } else {
        // 获取全部歌曲
        QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();
        // 获取当前搜索排序方式
//        DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
//                                             (DataBaseService::getInstance()->getPlaylistSortType("musicResult"));
        if (mediaMetas.size() > 0) {
            for (int i = 0; i < mediaMetas.size(); i++) {
                if (!CommonService::getInstance()->containsStr(musicName, mediaMetas.at(i).title)) {
                    continue;
                }
                m_MediaMetas.append(mediaMetas.at(i));
            }
            if (m_MediaMetas.size() > 0) {
                // 获得数据后排序
                // 不需要排序，播放的就是第一首
                //            sortList(m_MediaMetas, type);
                emit CommonService::getInstance()->signalSwitchToView(SearchMusicResultType, musicName);
                MediaMeta mediaMeta = m_MediaMetas.at(0);
                //重置播放队列
                Player::getInstance()->clearPlayList();
                Player::getInstance()->setPlayList(m_MediaMetas);
                //设置当前播放为查询出的音乐
                Player::getInstance()->setCurrentPlayListHash("musicResult", false);
                Player::getInstance()->playMeta(mediaMeta);
                // 通知播放队列刷新
                emit Player::getInstance()->signalPlayListChanged();
                str = m_settings->value("speechreply.speech.ok").toString();
            } else {
                // 没有歌曲
                str = m_settings->value("speechreply.speech.playMusicError").toString();
            }
        } else {
            // 没有歌曲
            str = m_settings->value("speechreply.speech.playMusicIsEmptyError").toString();
        }
    }
    m_needRefresh = false;
    return str;
}

// 指定歌手
QVariant SpeechCenter::playArtist(QString artistName)
{
    qDebug() << __FUNCTION__ << artistName;
    //查找该歌手
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    bool isExit = false;
    // 先完全匹配，匹配不到则模糊匹配
    for (int i = 0; i < singerInfos.size(); i++) {
        SingerInfo singerInfo = singerInfos[i];
        if (singerInfo.singerName == artistName && singerInfo.musicinfos.size() > 0) {
            // 找到歌手后跳转到歌手界面
            emit CommonService::getInstance()->signalSwitchToView(SingerType, "");
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(singerInfo.musicinfos.values());
            //设置当前播放为歌手
            Player::getInstance()->setCurrentPlayListHash("artist", false);
            //播放第一首
            Player::getInstance()->playMeta(singerInfo.musicinfos.values().first());
            emit CommonService::getInstance()->sigScrollToCurrentPosition("artist");
            // 通知播放队列刷新
            emit Player::getInstance()->signalPlayListChanged();
            isExit = true;
            break;
        }
    }
    if (!isExit) {
        for (int i = 0; i < singerInfos.size(); i++) {
            SingerInfo singerInfo = singerInfos[i];
            if (singerInfo.singerName.contains(artistName) && singerInfo.musicinfos.size() > 0) {
                // 找到歌手后跳转到歌手界面
                emit CommonService::getInstance()->signalSwitchToView(SingerType, "");
                //重置播放队列
                Player::getInstance()->clearPlayList();
                Player::getInstance()->setPlayList(singerInfo.musicinfos.values());
                //设置当前播放为歌手
                Player::getInstance()->setCurrentPlayListHash("artist", false);
                //播放第一首
                Player::getInstance()->playMeta(singerInfo.musicinfos.values().first());
                emit CommonService::getInstance()->sigScrollToCurrentPosition("artist");
                // 通知播放队列刷新
                emit Player::getInstance()->signalPlayListChanged();
                isExit = true;
                break;
            }
        }
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playArtistError").toString();
    }
    return str;
}

QVariant SpeechCenter::playArtistMusic(QString artistAndmusic)
{
    qDebug() << __FUNCTION__ << artistAndmusic;
    QStringList strList = artistAndmusic.split(":");
    QString artistName;
    QString musicName;
    if (strList.size() == 1) {
        artistName = strList.at(0);
    } else if (strList.size() == 2) {
        artistName = strList.at(0);
        musicName = strList.at(1);
    }
    //查找该歌手
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    bool isExit = false;
    for (SingerInfo singerInfo : singerInfos) {
        if (singerInfo.singerName == artistName && singerInfo.musicinfos.size() > 0) {
            // 找到歌手后跳转到歌手界面
            emit CommonService::getInstance()->signalSwitchToView(SingerType, "");
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(singerInfo.musicinfos.values());
            //设置当前播放为歌手
            Player::getInstance()->setCurrentPlayListHash("artist", false);

            // 如果没有歌曲名，播放第一首
            if (musicName.isEmpty()) {
                Player::getInstance()->playMeta(singerInfo.musicinfos.values().first());
                isExit = true;
                break;
            }
            // 有歌曲名，播放指定歌曲
            auto singerInfoMusicinfosValues = singerInfo.musicinfos.values();
            for (int i = 0; i < singerInfoMusicinfosValues.size(); i++) {
                MediaMeta meta = singerInfoMusicinfosValues[i];
                if (meta.title == musicName) {
                    Player::getInstance()->playMeta(meta);
                    isExit = true;
                    break;
                }
            }
            // 通知播放队列刷新
            emit Player::getInstance()->signalPlayListChanged();
            break;
        }
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playArtistMusicError").toString();
    }
    return str;
}

QVariant SpeechCenter::playAlbum(QString albumName)
{
    qDebug() << __FUNCTION__ << albumName;

    //查找该歌手
    QList<AlbumInfo> albumInfos = DataBaseService::getInstance()->allAlbumInfos();
    bool isExit = false;
    for (AlbumInfo albumInfo : albumInfos) {
        if (albumInfo.albumName.contains(albumName) && albumInfo.musicinfos.size() > 0) {
            // 找到专辑后跳转到专辑界面
            emit CommonService::getInstance()->signalSwitchToView(AlbumType, "");
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(albumInfo.musicinfos.values());
            //设置当前播放为歌手
            Player::getInstance()->setCurrentPlayListHash("album", false);
            // 有歌曲名与专辑名相同
            auto albumInfoMusicinfosValues = albumInfo.musicinfos.values();
            for (int i = 0; i < albumInfoMusicinfosValues.size(); i++) {
                MediaMeta meta = albumInfoMusicinfosValues[i];
                if (meta.title == albumName) {
                    Player::getInstance()->playMeta(meta);
                    isExit = true;
                    break;
                }
            }

            if (!isExit) {
                Player::getInstance()->playMeta(albumInfo.musicinfos.values().first());
                emit CommonService::getInstance()->sigScrollToCurrentPosition("album");
                isExit = true;
            }
            // 通知播放队列刷新
            emit Player::getInstance()->signalPlayListChanged();
            break;
        }
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playAlbumError").toString();
    }
    return str;
}

QVariant SpeechCenter::playFaverite(QString hash)
{
    Q_UNUSED(hash)
    qDebug() << __FUNCTION__ << hash;
    m_MediaMetas.clear();
    m_needRefresh = true;
    bool isExit = false;
    m_MediaMetas = DataBaseService::getInstance()->customizeMusicInfos("fav");
    if (m_MediaMetas.size() > 0) {
        // 收藏中有歌曲跳转到我的收藏
        emit CommonService::getInstance()->signalSwitchToView(FavType, "fav");
        MediaMeta mediaMeta = m_MediaMetas.at(0);
        //重置播放队列
        Player::getInstance()->clearPlayList();
        Player::getInstance()->setPlayList(m_MediaMetas);
        //设置当前播放为查询出的音乐
        Player::getInstance()->setCurrentPlayListHash("fav", false);
        Player::getInstance()->playMeta(mediaMeta);
        // 通知播放队列刷新
        emit Player::getInstance()->signalPlayListChanged();
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playFaveriteError").toString();
    }
    m_needRefresh = false;
    return str;
}

QVariant SpeechCenter::playSonglist(QString songlistName)
{
    // 去掉空格
    songlistName = songlistName.simplified();
    QString str;
    qDebug() << __FUNCTION__ << "songlistName = " << songlistName;
    QList<DataBaseService::PlaylistData> playlistDatas = DataBaseService::getInstance()->getCustomSongList();
    if (playlistDatas.size() <= 0) {
        // 没有自定义歌单
        str = m_settings->value("speechreply.speech.playSonglistNoSongList").toString();
    } else {
        QString uuid;
        if (songlistName.isEmpty()) {
            //过滤无歌曲的歌单
            QList<DataBaseService::PlaylistData> tmplist;
            for (DataBaseService::PlaylistData data : playlistDatas) {
                QList<MediaMeta> metalist = DataBaseService::getInstance()->customizeMusicInfos(data.uuid);
                if (metalist.size() > 0) {
                    tmplist << data;
                }
            }
            qDebug() << __FUNCTION__ << "song list to play size:" << tmplist.size();
            if (tmplist.size() > 0) {
                uuid = tmplist.at(0).uuid;
            }
            if (playlistDatas.size() > 0 && tmplist.size() <= 0) {
                // 没有自定义歌单
                str = m_settings->value("speechreply.speech.playSonglistNoSong").toString();
                m_needRefresh = false;
                return str;
            }
        } else {
            // 根据名称匹配歌单
            // 精确匹配
            if (songlistName == m_settings->value("speechreply.speech.songNameIsMyFav").toString()) {
                // 如果歌单名为“我喜爱的”，遍历歌单中有没有我喜爱的
                bool favExsit = false;
                for (int i = 0; i < playlistDatas.size(); i++) {
                    DataBaseService::PlaylistData playlistData = playlistDatas[i];
                    if (playlistData.displayName == songlistName) {
                        favExsit = true;
                        break;
                    }
                }
                if (!favExsit) {
                    return playFaverite("");
                }
            }
            for (int i = 0; i < playlistDatas.size(); i++) {
                DataBaseService::PlaylistData playlistData = playlistDatas[i];
                if (playlistData.displayName == songlistName) {
                    uuid = playlistData.uuid;
                    break;
                }
            }

            // 如果精确匹配没有则开始模糊匹配
            if (uuid.isEmpty()) {
                for (int i = 0; i < playlistDatas.size(); i++) {
                    DataBaseService::PlaylistData playlistData = playlistDatas[i];
                    if (playlistData.displayName.contains(songlistName)) {
                        uuid = playlistData.uuid;
                        break;
                    }
                }
            }
        }
        // 找到对应歌单
        if (!uuid.isEmpty()) {
            m_MediaMetas.clear();
            m_needRefresh = true;
            m_MediaMetas = DataBaseService::getInstance()->customizeMusicInfos(uuid);
            if (m_MediaMetas.size() > 0) {
                // 歌单中有歌曲跳转到自定义歌单
                emit CommonService::getInstance()->signalSwitchToView(CustomType, uuid);
                MediaMeta mediaMeta = m_MediaMetas.at(0);
                //重置播放队列
                Player::getInstance()->clearPlayList();
                Player::getInstance()->setPlayList(m_MediaMetas);
                //设置当前播放为查询出的音乐
                Player::getInstance()->setCurrentPlayListHash(uuid, false);
                Player::getInstance()->playMeta(mediaMeta);
                // 通知播放队列刷新
                emit Player::getInstance()->signalPlayListChanged();
                str = m_settings->value("speechreply.speech.ok").toString();
            } else {
                // 歌单中没有歌曲
                str = m_settings->value("speechreply.speech.playSonglistNoSong").toString();
            }
        } else {
            // 没有自定义歌单
            str = m_settings->value("speechreply.speech.playSonglistNoSongList").toString();
        }
    }
    m_needRefresh = false;
    return str;
}

QVariant SpeechCenter::pause(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (Player::getInstance()->status() == Player::PlaybackStatus::Playing) {
        Player::getInstance()->pause();
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
//        str = "抱歉，设置失败，请重新尝试。";
    }
    return str;
}

QVariant SpeechCenter::resume(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (Player::getInstance()->status() == Player::PlaybackStatus::Paused) {
        Player::getInstance()->resume();
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
//        str = "抱歉，设置失败，请重新尝试。";
    }
    return str;
}

QVariant SpeechCenter::stop(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (Player::getInstance()->status() == Player::PlaybackStatus::Playing) {
        Player::getInstance()->stop();
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
//        str = "抱歉，设置失败，请重新尝试。";
    }
    return str;
}

QVariant SpeechCenter::pre(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (Player::getInstance()->status() == Player::PlaybackStatus::Playing) {
        Player::getInstance()->playPreMeta();
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.preError").toString();
    }
    return str;
}

QVariant SpeechCenter::next(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (Player::getInstance()->status() == Player::PlaybackStatus::Playing) {
        Player::getInstance()->playNextMeta(false);
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.nextError").toString();
    }
    return str;
}

QVariant SpeechCenter::playIndex(QString index)
{
    qDebug() << __FUNCTION__ << "index = " << index;
    int indexNumber = index.toInt();
    bool isExit = false;
    QList<MediaMeta> mediaMetas = *Player::getInstance()->getPlayList();
    if (indexNumber > mediaMetas.size()) {
        isExit = false;
    } else {
        MediaMeta meta = mediaMetas.at(indexNumber - 1);
        Player::getInstance()->playMeta(meta);
        // 正确播放，重置标志位
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playIndexError").toString();
    }
    return str;
}

QVariant SpeechCenter::addFaverite(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (!Player::getInstance()->getActiveMeta().hash.isEmpty()) {
        QList<MediaMeta> listMeta;
        listMeta.append(Player::getInstance()->getActiveMeta());
        int count = DataBaseService::getInstance()->addMetaToPlaylist("fav", listMeta);
        if (count > 0) {
            isExit = true;
            emit CommonService::getInstance()->signalShowPopupMessage(
                DataBaseService::getInstance()->getPlaylistNameByUUID("fav"), 1, 1);
        }
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.addFaveriteOk").toString();
    } else {
//        str = "收藏失败。";
    }
    return str;
}

QVariant SpeechCenter::removeFaverite(QString musicName)
{
    Q_UNUSED(musicName)
    qDebug() << __FUNCTION__ << "";
    bool isExit = false;
    if (!Player::getInstance()->getActiveMeta().hash.isEmpty()) {
        QStringList listMeta;
        listMeta.append(Player::getInstance()->getActiveMeta().hash);
        int count = DataBaseService::getInstance()->deleteMetaFromPlaylist("fav", listMeta);
        if (count > 0) {
            isExit = true;
        }
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.removeFaveriteOk").toString();
    } else {
//        str = "收藏失败。";
    }
    return str;
}

QVariant SpeechCenter::setMode(QString mode)
{
    qDebug() << __FUNCTION__ << "";
    int modeNumber = mode.toInt();
    bool isExit = false;
    QString str;
    if (Player::getInstance()->status() == Player::PlaybackStatus::Playing) {
        if (modeNumber == Player::PlaybackMode::RepeatAll) {
            str = m_settings->value("speechreply.speech.setModeRepeatAllOk").toString();
            Player::getInstance()->setMode(Player::PlaybackMode::RepeatAll);
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
            isExit = true;
        } else if (modeNumber == Player::PlaybackMode::RepeatSingle) {
            str = m_settings->value("speechreply.speech.setModeRepeatSingleOk").toString();
            Player::getInstance()->setMode(Player::PlaybackMode::RepeatSingle);
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatSingle);
            isExit = true;
        } else if (modeNumber == Player::PlaybackMode::Shuffle) {
            str = m_settings->value("speechreply.speech.setModeShuffleOk").toString();
            Player::getInstance()->setMode(Player::PlaybackMode::Shuffle);
            emit CommonService::getInstance()->signalSetPlayModel(Player::Shuffle);
            isExit = true;
        }
    }
    if (!isExit) {
        str = "";
    }
    return str;
}

QVariant SpeechCenter::OpenUris(QVariant paths)
{
    qDebug() << __FUNCTION__ ;
    QStringList itemMetas;
    for (auto str : paths.value<QStringList>()) {
        QUrl url = QUrl(str);
        // 根据url类型进行转换
        if (url.isLocalFile()) {
            itemMetas << url.toLocalFile();
        } else {
            itemMetas << str;
        }
    }

    if (itemMetas.size() > 0) {
        DataBaseService::getInstance()->setFirstSong(itemMetas.first());
        DataBaseService::getInstance()->importMedias("all", itemMetas);
    }
    return true;
}

void SpeechCenter::setMediaMetas(const QList<MediaMeta> &mediaMetas)
{
    m_MediaMetas.clear();
    m_MediaMetas = mediaMetas;
}

bool SpeechCenter::getNeedRefresh()
{
    return m_needRefresh;
}
// 不需要排序，播放的就是第一首
//void SpeechCenter::sortList(QList<MediaMeta> &musicInfos, const DataBaseService::ListSortType &sortType)
//{
//    switch (sortType) {
//    case DataBaseService::SortByAddTimeASC: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.timestamp < v2.timestamp;
//        });
//        break;
//    }
//    case DataBaseService::SortByTitleASC: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.pinyinTitle < v2.pinyinTitle;
//        });
//        break;
//    }
//    case DataBaseService::SortBySingerASC: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.pinyinArtist < v2.pinyinArtist;
//        });
//        break;
//    }
//    case DataBaseService::SortByAblumASC: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.pinyinAlbum < v2.pinyinAlbum;
//        });
//        break;
//    }
//    case DataBaseService::SortByAddTimeDES: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.timestamp > v2.timestamp;
//        });
//        break;
//    }
//    case DataBaseService::SortByTitleDES: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.pinyinTitle > v2.pinyinTitle;
//        });
//        break;
//    }
//    case DataBaseService::SortBySingerDES: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.pinyinArtist > v2.pinyinArtist;
//        });
//        break;
//    }
//    case DataBaseService::SortByAblumDES: {
//        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
//            return v1.pinyinAlbum > v2.pinyinAlbum;
//        });
//        break;
//    }
//    default:
//        break;
//    }
//}
