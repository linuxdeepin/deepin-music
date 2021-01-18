#include "speechCenter.h"
#include "commonservice.h"
#include "databaseservice.h"
#include "player.h"
#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>

SpeechCenter::SpeechCenter(QObject *parent) : QObject(parent)
{
}
//指定歌曲
QVariant SpeechCenter::playMusic(QString musicName)
{
    qDebug() << __FUNCTION__ << musicName;
    //跳转到所有音乐
    m_MediaMetas.clear();
    m_needRefresh = true;
    bool isExit = false;
    if (musicName.isEmpty()) {
        emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "all");
        m_MediaMetas = DataBaseService::getInstance()->allMusicInfos();
        if (m_MediaMetas.size() > 0) {
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
            isExit = true;
        }
    } else {
        emit CommonService::getInstance()->signalSwitchToView(SearchMusicResultType, musicName);
        if (m_MediaMetas.size() > 0) {
            MediaMeta mediaMeta = m_MediaMetas.at(0);
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(m_MediaMetas);
            //设置当前播放为查询出的音乐
            Player::getInstance()->setCurrentPlayListHash("musicResult", false);
            Player::getInstance()->playMeta(mediaMeta);
            isExit = true;
        }
    }
    QString str;
    if (isExit) {
        str = "好的，没问题。";
    } else {
        str = "您的音乐播放器中没有歌曲，赶紧去添加吧。";
    }
    return str;
}

void SpeechCenter::setMediaMetas(const QList<MediaMeta> &metas)
{
    if (m_needRefresh) {
        m_MediaMetas = metas;
    }
    m_needRefresh = false;
}
// 指定歌手
QVariant SpeechCenter::playArtist(QString artistName)
{
    qDebug() << __FUNCTION__ << artistName;
    //跳转到歌手界面
    emit CommonService::getInstance()->signalSwitchToView(SingerType, "");
    //查找该歌手
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    bool isExit = false;
    for (SingerInfo singerInfo : singerInfos) {
        if (singerInfo.singerName == artistName && singerInfo.musicinfos.size() > 0) {
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(singerInfo.musicinfos.values());
            //设置当前播放为歌手
            Player::getInstance()->setCurrentPlayListHash("artist", false);
            //播放第一首
            Player::getInstance()->playMeta(singerInfo.musicinfos.values().first());
            emit CommonService::getInstance()->sigScrollToCurrentPosition("artist");
            isExit = true;
            break;
        }
    }
    QString str;
    if (isExit) {
        str = "好的，没问题。";
    } else {
        str = "抱歉，没有在音乐播放器中找到这首歌，您可以听其他音乐哦";
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
    //跳转到歌手界面
    emit CommonService::getInstance()->signalSwitchToView(SingerType, "");
    //查找该歌手
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    bool isExit = false;
    for (SingerInfo singerInfo : singerInfos) {
        if (singerInfo.singerName == artistName && singerInfo.musicinfos.size() > 0) {
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
            for (MediaMeta meta : singerInfo.musicinfos.values()) {
                if (meta.title == musicName) {
                    Player::getInstance()->playMeta(meta);
                    isExit = true;
                    break;
                }
            }
            break;
        }
    }
    QString str;
    if (isExit) {
        str = "好的，没问题。";
    } else {
        str = "抱歉，没有在音乐播放器中找到这首歌，您可以听其他音乐哦。";
    }
    return str;
}

QVariant SpeechCenter::playAlbum(QString albumName)
{
    qDebug() << __FUNCTION__ << albumName;
    //跳转到歌手界面
    emit CommonService::getInstance()->signalSwitchToView(AlbumType, "");
    //查找该歌手
    QList<AlbumInfo> albumInfos = DataBaseService::getInstance()->allAlbumInfos();
    bool isExit = false;
    for (AlbumInfo albumInfo : albumInfos) {
        if (albumInfo.albumName == albumName && albumInfo.musicinfos.size() > 0) {
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(albumInfo.musicinfos.values());
            //设置当前播放为歌手
            Player::getInstance()->setCurrentPlayListHash("album", false);
            // 有歌曲名与专辑名相同
            for (MediaMeta meta : albumInfo.musicinfos.values()) {
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
            break;
        }
    }
    QString str;
    if (isExit) {
        str = "好的，没问题。";
    } else {
        str = "您的音乐播放器中没有这首歌，换一首歌曲听听吧";
    }
    return str;
}

QVariant SpeechCenter::playFaverite(QString hash)
{
    Q_UNUSED(hash)
    qDebug() << __FUNCTION__ << hash;
    //跳转到所有音乐
    m_MediaMetas.clear();
    m_needRefresh = true;
    emit CommonService::getInstance()->signalSwitchToView(FavType, "fav");
    bool isExit = false;
    if (m_MediaMetas.size() > 0) {
        MediaMeta mediaMeta = m_MediaMetas.at(0);
        //重置播放队列
        Player::getInstance()->clearPlayList();
        Player::getInstance()->setPlayList(m_MediaMetas);
        //设置当前播放为查询出的音乐
        Player::getInstance()->setCurrentPlayListHash("fav", false);
        Player::getInstance()->playMeta(mediaMeta);
        isExit = true;
    }
    QString str;
    if (isExit) {
        str = "好的，没问题。";
    } else {
        str = "您的播放器中没有收藏的歌曲。";
    }
    return str;
}

QVariant SpeechCenter::playSonglist(QString songlistName)
{
    bool isExit = false;
    bool songlistExit = false;
    QString uuid;
    QList<DataBaseService::PlaylistData> playlistDatas = DataBaseService::getInstance()->getCustomSongList();
    for (DataBaseService::PlaylistData playlistData : playlistDatas) {
        if (playlistData.displayName == songlistName) {
            songlistExit = true;
            uuid = playlistData.uuid;
            break;
        }
    }
    if (songlistExit) {
        m_MediaMetas.clear();
        m_needRefresh = true;
        emit CommonService::getInstance()->signalSwitchToView(CustomType, uuid);
        if (m_MediaMetas.size() > 0) {
            MediaMeta mediaMeta = m_MediaMetas.at(0);
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(m_MediaMetas);
            //设置当前播放为查询出的音乐
            Player::getInstance()->setCurrentPlayListHash(uuid, false);
            Player::getInstance()->playMeta(mediaMeta);
            isExit = true;
        }
    } else {
        isExit = playFaverite("").toBool();
    }
    QString str;
    if (isExit) {
        str = "好的，没问题。";
    } else {
        str = "您的音乐播放器中没有这首歌，换一首歌曲听听吧。";
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
        str = "好的，没问题。";
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
        str = "好的，没问题。";
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
        str = "好的，没问题。";
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
        str = "好的，没问题。";
    } else {
        str = "非常抱歉，没有找到这首歌，你可以更换其他歌单试试。";
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
        str = "好的，没问题。";
    } else {
        str = "非常抱歉，没有找到这首歌，你可以更换其他歌单试试。";
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
        str = "好的，没问题。";
    } else {
        str = "非常抱歉，没有找到这首歌，你可以更换其他歌单试试。";
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
        str = "收藏成功。";
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
        str = "已取消收藏。";
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
            str = "已设为列表循环模式";
            Player::getInstance()->setMode(Player::PlaybackMode::RepeatAll);
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatAll);
            isExit = true;
        } else if (modeNumber == Player::PlaybackMode::RepeatSingle) {
            str = "已设为单曲循环模式";
            Player::getInstance()->setMode(Player::PlaybackMode::RepeatSingle);
            emit CommonService::getInstance()->signalSetPlayModel(Player::RepeatSingle);
            isExit = true;
        } else if (modeNumber == Player::PlaybackMode::Shuffle) {
            str = "已设为随机播放模式";
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
