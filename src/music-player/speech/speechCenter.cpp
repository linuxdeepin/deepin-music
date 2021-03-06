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
    bool isExit = false;
    if (musicName.isEmpty()) {
        m_MediaMetas = DataBaseService::getInstance()->allMusicInfos();
        if (m_MediaMetas.size() > 0) {
            emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "all");
            QTime time;
            int index = 0;
            time = QTime::currentTime();
            qsrand(static_cast<uint>((time.msec() + time.second() * 1000)));
            index = qrand() % m_MediaMetas.size();
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
            isExit = true;
        }
    } else {
        // 获取全部歌曲
        QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();
        // 获取当前搜索排序方式
        DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
                                             (DataBaseService::getInstance()->getPlaylistSortType("musicResult"));
        for (int i = 0; i < mediaMetas.size(); i++) {
            if (!CommonService::getInstance()->containsStr(musicName, mediaMetas.at(i).title)) {
                continue;
            }
            m_MediaMetas.append(mediaMetas.at(i));
        }
        if (m_MediaMetas.size() > 0) {
            // 获得数据后排序
            sortList(m_MediaMetas, type);
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
            isExit = true;
        }
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playMusicError").toString();
    }
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
    foreach (SingerInfo singerInfo, singerInfos) {
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
        foreach (SingerInfo singerInfo, singerInfos) {
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
            foreach (MediaMeta meta, singerInfo.musicinfos.values()) {
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
            foreach (MediaMeta meta, albumInfo.musicinfos.values()) {
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
    return str;
}

QVariant SpeechCenter::playSonglist(QString songlistName)
{
    bool isExit = false;
    bool songlistExit = false;
    QString uuid;
    QList<DataBaseService::PlaylistData> playlistDatas = DataBaseService::getInstance()->getCustomSongList();
    foreach (DataBaseService::PlaylistData playlistData, playlistDatas) {
        if (playlistData.displayName == songlistName) {
            songlistExit = true;
            uuid = playlistData.uuid;
            break;
        }
    }
    if (songlistExit) {
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
            isExit = true;
        }
    } else {
        isExit = playFaverite("").toBool();
    }
    QString str;
    if (isExit) {
        str = m_settings->value("speechreply.speech.ok").toString();
    } else {
        str = m_settings->value("speechreply.speech.playSonglistError").toString();
    }
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
    int indexNumber = index.toInt();
    bool isExit = false;
    QList<MediaMeta> mediaMetas = *Player::getInstance()->getPlayList();
    if (indexNumber > mediaMetas.size()) {
        isExit = false;
    } else {
        MediaMeta meta = mediaMetas.at(indexNumber - 1);
        Player::getInstance()->playMeta(meta);
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
    QStringList itemMetas = paths.value<QStringList>();

    if (itemMetas.size() > 0) {
        DataBaseService::getInstance()->setFirstSong(itemMetas.first());
        DataBaseService::getInstance()->importMedias("all", itemMetas);
    }
    return true;
}

void SpeechCenter::sortList(QList<MediaMeta> &musicInfos, const DataBaseService::ListSortType &sortType)
{
    switch (sortType) {
    case DataBaseService::SortByAddTimeASC: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.timestamp < v2.timestamp;
        });
        break;
    }
    case DataBaseService::SortByTitleASC: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.pinyinTitle < v2.pinyinTitle;
        });
        break;
    }
    case DataBaseService::SortBySingerASC: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.pinyinArtist < v2.pinyinArtist;
        });
        break;
    }
    case DataBaseService::SortByAblumASC: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.pinyinAlbum < v2.pinyinAlbum;
        });
        break;
    }
    case DataBaseService::SortByAddTimeDES: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.timestamp > v2.timestamp;
        });
        break;
    }
    case DataBaseService::SortByTitleDES: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.pinyinTitle > v2.pinyinTitle;
        });
        break;
    }
    case DataBaseService::SortBySingerDES: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.pinyinArtist > v2.pinyinArtist;
        });
        break;
    }
    case DataBaseService::SortByAblumDES: {
        qSort(musicInfos.begin(), musicInfos.end(), [](const MediaMeta v1, const MediaMeta v2) {
            return v1.pinyinAlbum > v2.pinyinAlbum;
        });
        break;
    }
    default:
        break;
    }
}
