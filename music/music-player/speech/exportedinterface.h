#ifndef EXPORTEDINTERFACE_H
#define EXPORTEDINTERFACE_H

#include <QObject>
#include <DExportedInterface>
DCORE_USE_NAMESPACE

//定义action
enum class ACTION {
    playMusic          = 1,  //播放歌曲名
    playArtist         = 2,  //播放歌手列表
    playArtistMusic    = 3,  //播放歌手歌曲名
    playFaverite       = 4,  //播放我的收藏
    playCustom         = 5,  //播放歌单
    playRadom          = 6,  //随机播放

    pause              = 11, //暂停
    stop               = 12, //停止
    resume             = 13, //继续播放
    previous           = 14, //上一首
    next               = 15, //下一首

    favorite           = 21, //收藏
    unFavorite         = 22, //取消收藏
    setMode            = 23  //播放模式
};

class SpeechCenter;
class ExportedInterface : public Dtk::Core::DUtil::DExportedInterface
{
//    Q_Ob
public:
    explicit ExportedInterface(QObject *parent = nullptr);
    QVariant invoke(const QString &action, const QString &parameters) const override;
private:
    QObject  *m_object;
    SpeechCenter *m_SpeechCenter;
};

#endif // EXPORTEDINTERFACE_H
