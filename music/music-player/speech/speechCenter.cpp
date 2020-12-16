#include "speechCenter.h"
#include "exportedinterface.h"
#include <QDebug>
//播放模式定义
enum class PLAYMODE {
    loop               = 1,  //循环播放
    single             = 2,  //单曲循环
    radom              = 3   //随机播放
};

SpeechCenter::SpeechCenter(QObject *parent) : QObject(parent)
{

}

bool SpeechCenter::playMusic(QString music)
{
    qDebug() << "-----playMusic:" << music;
    Q_EMIT sigPlayMusic(music);
    return playMusicResult;
}

bool SpeechCenter::playArtist(QString artist)
{
    qDebug() << "-----playArtist:" << artist;
    Q_EMIT sigPlayArtist(artist);
    return playArtistResult;
}

bool SpeechCenter::playArtistMusic(QString artist, QString music)
{
    qDebug() << "-----playArtistMusic artist:" << artist << "Music:" << music;
    Q_EMIT sigPlayArtistMusic(artist, music);
    return playArtistMusicResult;
}

bool SpeechCenter::playFaverite()
{
    qDebug() << "-----playFaverite";
    Q_EMIT sigPlayFaverite();
    return playFaveriteResult;
}

bool SpeechCenter::playCustom(QString listName)
{
    qDebug() << "-----playCustom:" << listName;
    Q_EMIT sigPlayCustom(listName);
    return playCustomResult;
}

bool SpeechCenter::playRadom()
{
    qDebug() << "-----playRadom:";
    Q_EMIT sigPlayRadom();
    return playRadomResult;
}

bool SpeechCenter::pause()
{
    qDebug() << "-----pause:";
    Q_EMIT sigPause();
    return true;
}

bool SpeechCenter::stop()
{
    qDebug() << "-----stop:";
    Q_EMIT sigStop();
    return true;
}

bool SpeechCenter::resume()
{
    qDebug() << "-----resume:";
    Q_EMIT sigResume();
    return true;
}

bool SpeechCenter::previous()
{
    qDebug() << "-----previous:";
    Q_EMIT sigPrevious();
    return true;
}

bool SpeechCenter::next()
{
    qDebug() << "-----next:";
    Q_EMIT sigNext();
    return true;
}

bool SpeechCenter::favorite()
{
    qDebug() << "-----set favorite:";
    Q_EMIT sigFavorite();
    return true;
}

bool SpeechCenter::unFaverite()
{
    qDebug() << "-----set unFaverite:";
    Q_EMIT unFaverite();
    return true;
}

bool SpeechCenter::setMode(int mode)
{
    qDebug() << "-----setMode:" << mode;
    Q_EMIT sigSetMode(mode);
    return true;
}

void SpeechCenter::onSpeedResult(int action, bool result)
{
    qDebug() << "-----onSpeedResult action" << action << "result" << result;
    ACTION mAction = ACTION(action);
    switch (mAction) {
    case ACTION::playMusic:
        playMusicResult = result;
        break;
    case ACTION::playArtist:
        playArtistResult = result;
        break;
    case ACTION::playArtistMusic:
        playArtistMusicResult = result;
        break;
    case ACTION::playFaverite:
        playFaveriteResult = result;
        break;
    case ACTION::playCustom:
        playCustomResult = result;
        break;
    case ACTION::playRadom:
        playRadomResult = result;
        break;
    default:
        break;
    }
}
