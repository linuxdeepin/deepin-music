#include "speechCenter.h"
#include "commonservice.h"
#include "databaseservice.h"
#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>
//播放模式定义
enum class PLAYMODE {
    loop               = 1,  //循环播放
    single             = 2,  //单曲循环
    radom              = 3   //随机播放
};

SpeechCenter::SpeechCenter(QObject *parent) : QObject(parent)
{

}
//指定歌曲
QVariant SpeechCenter::playMusic(QString musicName)
{
    qDebug() << __FUNCTION__ << musicName;
    //跳转到所有音乐
    emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "");
    //查找该歌手
    QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();
    bool isExit = false;
    for (MediaMeta mediaMeta : mediaMetas) {
        if (mediaMeta.title == musicName) {
            //重置播放队列
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(mediaMetas);
            //设置当前播放为所有音乐
            Player::getInstance()->setCurrentPlayListHash("all", false);
            Player::getInstance()->playMeta(mediaMeta);
            emit CommonService::getInstance()->sigScrollToCurrentPosition("all");
            isExit = true;
            break;
        }
    }
    return isExit;
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
    return isExit;
}

QVariant SpeechCenter::playArtistMusic(QString artistAndmusic)
{
//    qDebug() << "-----playArtistMusic artist:" << artist << "Music:" << music;
}

//bool SpeechCenter::playFaverite()
//{
//    qDebug() << "-----playFaverite";
//    Q_EMIT sigPlayFaverite();
//    return playFaveriteResult;
//}

//bool SpeechCenter::playCustom(QString listName)
//{
//    qDebug() << "-----playCustom:" << listName;
//    Q_EMIT sigPlayCustom(listName);
//    return playCustomResult;
//}

//bool SpeechCenter::playRadom()
//{
//    qDebug() << "-----playRadom:";
//    Q_EMIT sigPlayRadom();
//    return playRadomResult;
//}

//bool SpeechCenter::pause()
//{
//    qDebug() << "-----pause:";
//    Q_EMIT sigPause();
//    return true;
//}

//bool SpeechCenter::stop()
//{
//    qDebug() << "-----stop:";
//    Q_EMIT sigStop();
//    return true;
//}

//bool SpeechCenter::resume()
//{
//    qDebug() << "-----resume:";
//    Q_EMIT sigResume();
//    return true;
//}

//bool SpeechCenter::previous()
//{
//    qDebug() << "-----previous:";
//    Q_EMIT sigPrevious();
//    return true;
//}

//bool SpeechCenter::next()
//{
//    qDebug() << "-----next:";
//    Q_EMIT sigNext();
//    return true;
//}

//bool SpeechCenter::favorite()
//{
//    qDebug() << "-----set favorite:";
//    Q_EMIT sigFavorite();
//    return true;
//}

//bool SpeechCenter::unFaverite()
//{
//    qDebug() << "-----set unFaverite:";
//    Q_EMIT unFaverite();
//    return true;
//}

//bool SpeechCenter::setMode(int mode)
//{
//    qDebug() << "-----setMode:" << mode;
//    Q_EMIT sigSetMode(mode);
//    return true;
//}

//void SpeechCenter::onSpeedResult(int action, bool result)
//{
//    qDebug() << "-----onSpeedResult action" << action << "result" << result;
//    ACTION mAction = ACTION(action);
//    switch (mAction) {
//    case ACTION::playMusic:
//        playMusicResult = result;
//        break;
//    case ACTION::playArtist:
//        playArtistResult = result;
//        break;
//    case ACTION::playArtistMusic:
//        playArtistMusicResult = result;
//        break;
//    case ACTION::playFaverite:
//        playFaveriteResult = result;
//        break;
//    case ACTION::playCustom:
//        playCustomResult = result;
//        break;
//    case ACTION::playRadom:
//        playRadomResult = result;
//        break;
//    default:
//        break;
//    }
//}
