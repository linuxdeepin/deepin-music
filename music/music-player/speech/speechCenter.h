#ifndef SPEECHCENTER_H
#define SPEECHCENTER_H

#include "util/singleton.h"

#include <QObject>

class SpeechCenter : public QObject, public DMusic::DSingleton<SpeechCenter>
{
    Q_OBJECT
public:
    static QVariant playMusic(QString musicName);
    static QVariant playArtist(QString artistName);
    static QVariant playArtistMusic(QString artistAndmusic);
//    bool playFaverite();
//    bool playCustom(QString listName);
//    bool playRadom();

//    bool pause();
//    bool stop();
//    bool resume();
//    bool previous();
//    bool next();

//    bool favorite();
//    bool unFaverite();
//    bool setMode(int mode);

signals:
    void sigPlayMusic(QString music);
    void sigPlayFaverite();
    void sigPlayCustom(QString listName);
    void sigPlayRadom();

    void sigPause();
    void sigStop();
    void sigResume();
    void sigPrevious();
    void sigNext();

    void sigFavorite();
    void sigUnFaverite();
    void sigSetMode(int mode);
public slots:
//    void onSpeedResult(int action, bool result);
private:
    explicit SpeechCenter(QObject *parent = nullptr);
    friend class DMusic::DSingleton<SpeechCenter>;

private:
    bool playMusicResult        = true;
    bool playArtistMusicResult  = true;
    bool playFaveriteResult     = true;
    bool playCustomResult       = true;
    bool playRadomResult        = true;
};

#endif // SPEECHCENTER_H
