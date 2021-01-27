#ifndef SPEECHCENTER_H
#define SPEECHCENTER_H
#include <QObject>

#include <DSettings>

#include "util/singleton.h"
#include "mediameta.h"
#include "databaseservice.h"

class SpeechCenter : public QObject, public DMusic::DSingleton<SpeechCenter>
{
    Q_OBJECT
public:
    explicit SpeechCenter(QObject *parent = nullptr);
    // 播放指定歌曲,若musicName为空则随机播放一首
    QVariant playMusic(QString musicName);
    // 播放歌手
    QVariant playArtist(QString artistName);
    // 播放歌手指定歌曲
    QVariant playArtistMusic(QString artistAndmusic);
    // 播放专辑
    QVariant playAlbum(QString albumName);
    // 播放我的收藏,该参数暂时不使用
    QVariant playFaverite(QString hash);
    // 播放歌单
    QVariant playSonglist(QString songlistName);

    // 暂停
    QVariant pause(QString musicName);
    // 继续播放
    QVariant resume(QString musicName);
    // 停止
    QVariant stop(QString musicName);
    // 上一首
    QVariant pre(QString musicName);
    // 下一首
    QVariant next(QString musicName);
    // 指定播放第几首
    QVariant playIndex(QString index);
    // 添加收藏
    QVariant addFaverite(QString musicName);
    // 取消收藏
    QVariant removeFaverite(QString musicName);
    // 设置播放模式
    QVariant setMode(QString mode);
    // 音乐打开后通过dbus导入文件
    QVariant OpenUris(QVariant paths);
private:
    // 排序
    void sortList(QList<MediaMeta> &musicInfos, const DataBaseService::ListSortType &sortType);
private:
    friend class DMusic::DSingleton<SpeechCenter>;
private:
    bool m_needRefresh;
    QList<MediaMeta>  m_MediaMetas;
    Dtk::Core::DSettings *m_settings = nullptr;
};

#endif // SPEECHCENTER_H
