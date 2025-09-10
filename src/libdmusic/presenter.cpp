// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "presenter.h"
#include "util/log.h"

#include <QDebug>
#include <QColor>
#include <QFileInfo>
#include <QDir>
#include <QDBusInterface>
#include <QCoreApplication>

#include "playerengine.h"
#include "lyricanalysis.h"
#include "ckmeans.h"
#include "datamanager.h"
#include "utils.h"
#include "audioanalysis.h"

class PresenterPrivate
{
public:
    PresenterPrivate(Presenter *parent)
        : m_parent(parent)
    {
        qCDebug(dmMusic) << "PresenterPrivate constructor";
        m_playerEngine = new PlayerEngine(m_parent);
        m_dataManager = new DataManager(m_playerEngine->supportedSuffixList(), m_parent);
        m_playerEngine->setPlaybackMode((DmGlobal::PlaybackMode)m_dataManager->valueFromSettings("base.play.playmode").toInt());
        m_pkmeans = new CKMeans;
        m_playerEngine->addMetasToPlayList(m_dataManager->getPlaylistMetas("play"));
        m_audioAnalysis = new AudioAnalysis(m_parent);
    }
    ~PresenterPrivate()
    {
        qCDebug(dmMusic) << "PresenterPrivate destructor";
        m_audioAnalysis->stopRecorder();
        // 颜色聚类
        if (m_pkmeans) {
            qCDebug(dmMusic) << "PresenterPrivate destructor delete m_pkmeans";
            delete m_pkmeans;
            m_pkmeans = nullptr;
        }
    }
private:
    friend class Presenter;
    Presenter                  *m_parent;
    PlayerEngine               *m_playerEngine     = nullptr;
    CKMeans                    *m_pkmeans          = nullptr;
    DataManager                *m_dataManager      = nullptr;
    AudioAnalysis              *m_audioAnalysis    = nullptr;
    LyricAnalysis               m_lyricAnalysis;
};

Presenter::Presenter(const QString &unknownAlbumStr, const QString &unknownArtistStr,
                     QObject *parent)
    : QObject(parent)
{
    qCInfo(dmMusic) << "Initializing Presenter";
    qRegisterMetaType<DMusic::MediaMeta>("DMusic::MediaMeta");
    qRegisterMetaType<DMusic::PlaylistInfo>("DMusic::PlaylistInfo");
    qRegisterMetaType<DMusic::AlbumInfo>("DMusic::AlbumInfo");
    qRegisterMetaType<DMusic::ArtistInfo>("DMusic::ArtistInfo");
    DmGlobal::initPath();
    DmGlobal::setUnknownAlbumText(unknownAlbumStr);
    DmGlobal::setUnknownArtistText(unknownArtistStr);

    m_data = new PresenterPrivate(this);
    
    qCDebug(dmMusic) << "Setting up signal connections";
    
    connect(m_data->m_playerEngine, &PlayerEngine::volumeChanged, this, &Presenter::volumeChanged);
    connect(m_data->m_playerEngine, &PlayerEngine::muteChanged, this, &Presenter::muteChanged);
    connect(m_data->m_playerEngine, &PlayerEngine::metaChanged, this, [ = ]() {
        qCDebug(dmMusic) << "Meta changed, updating audio buffer and settings";
        m_data->m_audioAnalysis->parseAudioBuffer(m_data->m_playerEngine->getMediaMeta());
        m_data->m_dataManager->setValueToSettings("base.play.last_meta", m_data->m_playerEngine->getMediaMeta().hash, true);
        emit metaChanged();
    });
    connect(m_data->m_playerEngine, &PlayerEngine::playPictureChanged, this, &Presenter::updatePlayingIcon);
    connect(m_data->m_playerEngine, &PlayerEngine::positionChanged, this, &Presenter::positionChanged);
    connect(m_data->m_playerEngine, &PlayerEngine::playbackStatusChanged, this, [ = ](DmGlobal::PlaybackStatus status) {
        qCDebug(dmMusic) << "Playback status changed to:" << status;
        if (status == DmGlobal::Playing) {
            qCDebug(dmMusic) << "Starting audio recorder";
            m_data->m_audioAnalysis->startRecorder();
        } else {
            qCDebug(dmMusic) << "Suspending audio recorder";
            m_data->m_audioAnalysis->suspendRecorder();
        }
        emit playbackStatusChanged(status);
    });
    connect(m_data->m_playerEngine, &PlayerEngine::sendCdaStatus, this, [ = ](int state) {
        qCDebug(dmMusic) << "CD audio status changed to:" << state;
        if (state == -1) {
            qCInfo(dmMusic) << "Removing all CD tracks from playlist";
            QList<QString> allCDHashs;
            for (auto meta : m_data->m_playerEngine->getMetas()) {
                if (meta.filetype == "cdda") {
                    deleteOneMeta(QStringList() << "play", meta.hash);
                    allCDHashs.append(meta.hash);
                }
            }
            m_data->m_playerEngine->removeMetasFromPlayList(allCDHashs);
            deleteFinished(QStringList() << "play");
            deletedPlaylist("cdarole");
        } else {
            qCDebug(dmMusic) << "Updating CD status to:" << state;
            emit updateCDStatus(state);
        }
    });
    connect(m_data->m_playerEngine, &PlayerEngine::playPlaylistRequested, this, [this](const QString &playlistHash){
        qCDebug(dmMusic) << "Playlist play requested for hash:" << playlistHash;
        playPlaylist(playlistHash);
    });
    connect(m_data->m_playerEngine, &PlayerEngine::quitRequested, this, &Presenter::quitRequested);
    connect(m_data->m_playerEngine, &PlayerEngine::raiseRequested, this, &Presenter::raiseRequested);

    connect(m_data->m_audioAnalysis, &AudioAnalysis::audioSpectrumData, this, &Presenter::audioSpectrumData);
    connect(m_data->m_audioAnalysis, &AudioAnalysis::audioBuffer, this, [ = ](const QVector<float> &buffer, const QString & hash) {
        QVariantList list;
        for (int i = 0; i < buffer.size(); i++) {
            list.append(buffer[i]);
        }
        emit audioBuffer(list, hash);
    });

    connect(m_data->m_dataManager, &DataManager::signalCurrentPlaylistSChanged, this, &Presenter::currentPlaylistSChanged);
    connect(m_data->m_dataManager, &DataManager::signalPlaylistSortChanged, this, &Presenter::playlistSortChanged);
    connect(m_data->m_dataManager, &DataManager::signalAddMetaFinished, this, &Presenter::addMetaFinished);
    connect(m_data->m_dataManager, &DataManager::signalAddOneMeta, this,
            [ = ](QStringList playlistHashs, DMusic::MediaMeta meta, const bool & addToPlay) {
        qCDebug(dmMusic) << "Adding meta to playlists:" << playlistHashs << "Meta hash:" << meta.hash;
        if (playlistHashs.contains("play") && addToPlay) {
            m_data->m_playerEngine->addMetasToPlayList(QList<DMusic::MediaMeta>() << meta);
        }
        emit addOneMeta(playlistHashs, Utils::metaToVariantMap(meta));
    });
    connect(m_data->m_dataManager, &DataManager::signalImportFinished, this, [ = ](QStringList playlistHashs, int failCount, int sucessCount, int existCount, QString mediaHash) {
        // 自动播放
        if (!mediaHash.isEmpty()) {
            if (m_data->m_playerEngine->getMediaMeta().hash != mediaHash)
                m_data->m_playerEngine->setMediaMeta(mediaHash);
            m_data->m_playerEngine->play();
        }
        
        // 导入歌曲成功后保存数据库
        if (sucessCount > 0) {
            qCInfo(dmMusic) << "Saving database after importing" << sucessCount << "songs";
            saveDataToDB();
        }
        
        emit importFinished(playlistHashs, failCount, sucessCount, existCount);
    });
    connect(m_data->m_dataManager, &DataManager::signalDeleteOneMeta, this,
            [ = ](QStringList playlistHashs, QString hash, const bool & addToPlay) {
        if (playlistHashs.contains("play") && addToPlay) m_data->m_playerEngine->removeMetaFromPlayList(hash);
        emit deleteOneMeta(playlistHashs, hash);
    });
    connect(m_data->m_dataManager, &DataManager::signalDeleteFinished, this, &Presenter::deleteFinished);
    connect(m_data->m_dataManager, &DataManager::signalUpdatedMetaCodec, this, [ = ](DMusic::MediaMeta meta, QString preAlbum, QString preArtist) {
        emit updatedMetaCodec(Utils::metaToVariantMap(meta), preAlbum, preArtist);
    });

    connect(this, &Presenter::restorePlaybackStatus, this, [ = ]() {
        // 初始化歌曲
        if (!m_data->m_dataManager->valueFromSettings("base.play.last_meta").toString().isEmpty()) {
            m_data->m_playerEngine->setMediaMeta(m_data->m_dataManager->valueFromSettings("base.play.last_meta").toString());
        }
        m_data->m_playerEngine->setFadeInOut(m_data->m_dataManager->valueFromSettings("base.play.fade_in_out").toBool());
        m_data->m_playerEngine->setVolume(m_data->m_dataManager->valueFromSettings("base.play.volume").toInt());
        m_data->m_playerEngine->setMute(m_data->m_dataManager->valueFromSettings("base.play.mute").toBool());
        if (m_data->m_dataManager->valueFromSettings("base.play.remember_progress").toBool()
                && !m_data->m_dataManager->valueFromSettings("base.play.last_meta").toString().isEmpty()) {
            m_data->m_playerEngine->setTime(m_data->m_dataManager->valueFromSettings("base.play.last_position").toInt());
        }
        if (m_data->m_dataManager->valueFromSettings("base.play.auto_play").toBool()) {
            m_data->m_playerEngine->play();
        }
        if (!m_data->m_dataManager->valueFromSettings("base.play.last_playlist").toString().isEmpty()) {
            setCurrentPlayList(m_data->m_dataManager->valueFromSettings("base.play.last_playlist").toString());
        }

        //读取均衡器使能开关配置
        auto eqSwitch = m_data->m_dataManager->valueFromSettings("equalizer.all.switch").toBool();
        if (eqSwitch) {
            qCDebug(dmMusic) << "Equalizer switch is enabled";
            //载入当前设置音效
            auto curIndex = m_data->m_dataManager->valueFromSettings("equalizer.all.curEffect").toInt();
            //非自定义模式时
            if (curIndex > 0) {
                qCDebug(dmMusic) << "Equalizer preset index:" << curIndex;
                m_data->m_playerEngine->loadFromPreset(uint(curIndex - 1));
                //设置放大值
                m_data->m_playerEngine->setPreamplification(m_data->m_playerEngine->preamplification());
                for (int i = 0 ; i < 10; i++) {
                    //设置频率值
                    m_data->m_playerEngine->setAmplificationForBandAt(m_data->m_playerEngine->amplificationForBandAt(uint(i)), uint(i));
                }
            } else {
                qCDebug(dmMusic) << "Equalizer custom mode";
                //自定义频率
                QList<int > allBauds;
                allBauds.clear();
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_pre").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_60").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_170").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_310").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_600").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_1K").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_3K").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_6K").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_12K").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_14K").toInt());
                allBauds.append(m_data->m_dataManager->valueFromSettings("equalizer.all.baud_16K").toInt());
                m_data->m_playerEngine->setPreamplification(allBauds.at(0));
                for (int i = 1; i < 11; i++) {
                    m_data->m_playerEngine->setAmplificationForBandAt(allBauds.at(i), uint(i - 1));
                }
            }
        }
    });
    
    qCInfo(dmMusic) << "Presenter initialization completed";
}

Presenter::~Presenter()
{
    qCDebug(dmMusic) << "Presenter destructor";
    if (m_data) {
        qCDebug(dmMusic) << "Presenter destructor delete m_data";
        delete m_data;
        m_data = nullptr;
    }
}

void Presenter::setMprisPlayer(const QString &serviceName, const QString &desktopEntry, const QString &identity)
{
    qCDebug(dmMusic) << "Setting MPRIS player - Service:" << serviceName
                     << "Desktop Entry:" << desktopEntry
                     << "Identity:" << identity;
    m_data->m_playerEngine->setMprisPlayer(serviceName, desktopEntry, identity);
}

QStringList Presenter::supportedSuffixList() const
{
    qCDebug(dmMusic) << "Getting supported file suffixes";
    QStringList suffixList;
    for (QString str : m_data->m_playerEngine->supportedSuffixList()) {
        suffixList.append("*." + str);
    }
    qCDebug(dmMusic) << "Supported suffixes:" << suffixList;
    return suffixList;
}

QColor Presenter::getMainColorByKmeans()
{
    qCDebug(dmMusic) << "Getting main color from KMeans clustering";
    return m_data->m_pkmeans->getCommColorMain();
}

QColor Presenter::getSecondColorByKmeans()
{
    qCDebug(dmMusic) << "Getting second color from KMeans clustering";
    return m_data->m_pkmeans->getCommColorSecond();
}

QImage Presenter::getEffectImage()
{
    qCDebug(dmMusic) << "Getting effect image from KMeans clustering";
    return m_data->m_pkmeans->getShowImage();
}

void Presenter::setEffectImage(const QImage &img)
{
    qCDebug(dmMusic) << "Setting effect image in KMeans clustering";
    m_data->m_pkmeans->setShowImage(img);
}

void Presenter::forceExit()
{
    qCInfo(dmMusic) << "Force exit requested";
    saveDataToDB();
    qApp->processEvents();
    QCoreApplication::exit(0);
    qCDebug(dmMusic) << "Forcing immediate exit";
    _Exit(0);
}

QVariantList Presenter::getLyrics()
{
    qCDebug(dmMusic) << "Getting lyrics";
    QVariantList lyrics;
    DMusic::MediaMeta meta = m_data->m_playerEngine->getMediaMeta();
    if (!meta.localPath.isEmpty()) {
        qCDebug(dmMusic) << "Local path of media:" << meta.localPath;
        QFileInfo fileInfo(meta.localPath);
        QString lrcPath = fileInfo.dir().path() + QDir::separator() + fileInfo.completeBaseName() + ".lrc";
        // 同目录下歌词文件不存在，读取缓存中解析的歌词
        QFile file(lrcPath);
        if (!file.exists()) {
            qCDebug(dmMusic) << "Lyrics file does not exist, searching in cache";
            lrcPath = DmGlobal::cachePath() + QDir::separator() + "lyrics" + QDir::separator() + meta.hash + ".lrc";
        }
        m_data->m_lyricAnalysis.setFromFile(lrcPath);
        QVector<QPair<qint64, QString> > allLyrics = m_data->m_lyricAnalysis.allLyrics();
        for (QPair<qint64, QString> lyric : allLyrics) {
            QVariantMap curData;
            curData.insert("time", lyric.first);
            curData.insert("lyric", lyric.second);
            lyrics.append(curData);
        }
    }
    qCDebug(dmMusic) << "Returning lyrics:" << lyrics.size();
    return lyrics;
}

void Presenter::setActivateMeta(const QString &metaHash)
{
    qCDebug(dmMusic) << "Setting active media meta:" << metaHash;
    m_data->m_playerEngine->setMediaMeta(metaHash);
}

QImage Presenter::getActivateMetImage()
{
    qCDebug(dmMusic) << "Getting active media cover image";
    return AudioAnalysis::getMetaCoverImage(m_data->m_playerEngine->getMediaMeta());
}

QVariantMap Presenter::getActivateMeta()
{
    qCDebug(dmMusic) << "Getting active media meta";
    return Utils::metaToVariantMap(m_data->m_playerEngine->getMediaMeta());
}

QVariant Presenter::getPlaybackStatus()
{
    qCDebug(dmMusic) << "Getting playback status";
    return QVariant(m_data->m_playerEngine->playbackStatus());
}

void Presenter::setMute(bool mute)
{
    qCDebug(dmMusic) << "Setting mute state:" << mute;
    m_data->m_playerEngine->setMute(mute);
}

bool Presenter::getMute()
{
    qCDebug(dmMusic) << "Getting mute state";
    return m_data->m_playerEngine->getMute();
}

void Presenter::setVolume(int volume)
{
    qCDebug(dmMusic) << "Setting volume:" << volume;
    m_data->m_playerEngine->setVolume(volume);
}

int Presenter::getVolume()
{
    qCDebug(dmMusic) << "Getting volume";
    return m_data->m_playerEngine->getVolume();
}

void Presenter::setPlaybackMode(QVariant mode)
{
    qCDebug(dmMusic) << "Setting playback mode";
    m_data->m_playerEngine->setPlaybackMode(mode.value<DmGlobal::PlaybackMode>());
    m_data->m_dataManager->setValueToSettings("base.play.playmode", mode.value<DmGlobal::PlaybackMode>());
}

QVariant Presenter::getPlaybackMode()
{
    qCDebug(dmMusic) << "Getting playback mode";
    return QVariant(m_data->m_playerEngine->getPlaybackMode());
}

void Presenter::setPosition(qint64 position)
{
    qCDebug(dmMusic) << "Setting position:" << position;
    m_data->m_playerEngine->setTime(position);
}

qint64 Presenter::getPosition()
{
    qCDebug(dmMusic) << "Getting position";
    return m_data->m_playerEngine->time();
}

void Presenter::setEQ(bool enabled, int curIndex, QVariantList indexbaud)
{
    qCDebug(dmMusic) << "Setting EQ state:" << enabled;
    if (enabled) {
        qCDebug(dmMusic) << "EQ enabled, loading preset";
        //非自定义模式时
        if (curIndex > 0) {
            qCDebug(dmMusic) << "Loading preset with index:" << curIndex - 1;
            m_data->m_playerEngine->loadFromPreset(uint(curIndex - 1));
            //设置放大值
            m_data->m_playerEngine->setPreamplification(m_data->m_playerEngine->preamplification());
            for (int i = 0 ; i < 10; i++) {
                //设置频率值
                m_data->m_playerEngine->setAmplificationForBandAt(m_data->m_playerEngine->amplificationForBandAt(uint(i)), uint(i));
            }
        } else {
            qCDebug(dmMusic) << "Custom EQ preset, setting amplification";

            if (indexbaud.size() == 0) {
                qCDebug(dmMusic) << "No amplification values provided, returning";
                return;
            } else {
                qCDebug(dmMusic) << "Setting preamplification to:" << indexbaud.at(0).toInt();
                m_data->m_playerEngine->setPreamplification(indexbaud.at(0).toInt());
                for (int i = 1; i < 11; i++) {
                    m_data->m_playerEngine->setAmplificationForBandAt(indexbaud.at(i).toInt(), uint(i - 1));
                }
            }
        }
    }
    qCDebug(dmMusic) << "EQ settings applied";
}

void Presenter::setEQEnable(bool enable)
{
    qCDebug(dmMusic) << "Setting EQ enable state:" << enable;
    m_data->m_playerEngine->setEqualizerEnabled(enable);
}

void Presenter::setEQpre(int val)
{
    qCDebug(dmMusic) << "Setting preamplification to:" << val;
    m_data->m_playerEngine->setPreamplification(val);
}

void Presenter::setEQbauds(int index, int val)
{
    qCDebug(dmMusic) << "Setting amplification for band at index:" << index << "to:" << val;
    m_data->m_playerEngine->setAmplificationForBandAt(uint(val), uint(index));
}

void Presenter::setEQCurMode(int curIndex)
{
    qCDebug(dmMusic) << "Setting EQ current mode to:" << curIndex;
    //非自定义模式时
    if (curIndex != 0) {
        qCDebug(dmMusic) << "Loading preset with index:" << curIndex - 1;
        m_data->m_playerEngine->loadFromPreset(uint(curIndex - 1));
        //设置放大值
        m_data->m_playerEngine->setPreamplification(m_data->m_playerEngine->preamplification());
        for (int i = 0 ; i < 10; i++) {
            //设置频率值
            m_data->m_playerEngine->setAmplificationForBandAt(m_data->m_playerEngine->amplificationForBandAt(uint(i)), uint(i));
        }
    }
    qCDebug(dmMusic) << "EQ settings applied";
}

void Presenter::showMetaFile(const QString &hash)
{
    qCDebug(dmMusic) << "Displaying metadata for hash:" << hash;
    DMusic::MediaMeta meta = m_data->m_dataManager->metaFromHash(hash);
    if (meta.localPath.isEmpty()) return;

    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                             QStringLiteral("/org/freedesktop/FileManager1"),
                             QStringLiteral("org.freedesktop.FileManager1"));
    interface.call("ShowItems", QStringList() << meta.localPath, QString());
}

bool Presenter::nextMetaFromPlay(const QString &metaHash)
{
    qCDebug(dmMusic) << "Playing next track after:" << metaHash;
    bool flag = false, hasMeta = false;
    QList<DMusic::MediaMeta> allMetas = m_data->m_dataManager->getPlaylistMetas("play");
    for (int i = 0; i < allMetas.size(); i++) {
        if ((hasMeta || allMetas[i].hash == metaHash) && ((QFile::exists(allMetas[i].localPath) && m_data->m_playerEngine->supportedSuffixList().contains(
                                                               QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA)) {
            if (hasMeta) {
                flag = true;
                break;
            } else {
                hasMeta = true;
            }
        }
    }
    qCDebug(dmMusic) << "Next track found:" << flag;
    return flag;
}

bool Presenter::preMetaFromPlay(const QString &metaHash)
{
    qCDebug(dmMusic) << "Playing previous track before:" << metaHash;
    bool flag = false, hasMeta = false;
    QList<DMusic::MediaMeta> allMetas = m_data->m_dataManager->getPlaylistMetas("play");
    for (int i = allMetas.size() - 1; i >= 0; i--) {
        if ((hasMeta || allMetas[i].hash == metaHash) && ((QFile::exists(allMetas[i].localPath) && m_data->m_playerEngine->supportedSuffixList().contains(
                                                               QFileInfo(allMetas[i].localPath).suffix().toLower())) || allMetas[i].mmType == DmGlobal::MimeTypeCDA)) {
            if (hasMeta) {
                flag = true;
                break;
            } else {
                hasMeta = true;
            }
        }
    }
    qCDebug(dmMusic) << "Previous track found:" << flag;
    return flag;
}

void Presenter::playAlbum(const QString &album, const QString &metaHash)
{
    qCInfo(dmMusic) << "Playing album:" << album << "Starting with track:" << metaHash;
    bool playFlag = m_data->m_playerEngine->getMediaMeta().hash != metaHash;
    QList<DMusic::AlbumInfo> albums = m_data->m_dataManager->allAlbumInfos();
    QList<DMusic::MediaMeta> allMetas;
    
    for (const DMusic::AlbumInfo &curAlbum : albums) {
        if (curAlbum.name == album) {
            allMetas += curAlbum.musicinfos.values();
            for (auto meta : allMetas) {
                if (m_data->m_playerEngine->getMediaMeta().hash == meta.hash) {
                    playFlag = false;
                    break;
                }
            }
            break;
        }
    }

    if (allMetas.isEmpty()) {
        qCWarning(dmMusic) << "No tracks found for album:" << album;
        return;
    }

    qCDebug(dmMusic) << "Loading" << allMetas.size() << "tracks from album";
    m_data->m_playerEngine->clearPlayList(playFlag);
    m_data->m_playerEngine->addMetasToPlayList(allMetas);
    
    if (!metaHash.isEmpty() && m_data->m_playerEngine->getMediaMeta().hash != metaHash) {
        qCDebug(dmMusic) << "Setting start track to:" << metaHash;
        m_data->m_playerEngine->setMediaMeta(metaHash);
    }
    
    m_data->m_playerEngine->setCurrentPlayList("album");
    m_data->m_playerEngine->play();
    m_data->m_dataManager->setCurrentPlayliHash("album");
    m_data->m_dataManager->clearPlayList("play", false);
    m_data->m_dataManager->addMetasToPlayList(allMetas, "play", false);
    
    qCInfo(dmMusic) << "Album playback started successfully";
}

void Presenter::playArtist(const QString &artist, const QString &metaHash)
{
    qCInfo(dmMusic) << "Playing artist:" << artist << "Starting with track:" << metaHash;
    bool playFlag = m_data->m_playerEngine->getMediaMeta().hash != metaHash;
    QList<DMusic::ArtistInfo> artists = m_data->m_dataManager->allArtistInfos();
    QList<DMusic::MediaMeta> allMetas;
    
    for (const DMusic::ArtistInfo &curArtist : artists) {
        if (curArtist.name == artist) {
            allMetas += curArtist.musicinfos.values();
            for (auto meta : allMetas) {
                if (m_data->m_playerEngine->getMediaMeta().hash == meta.hash) {
                    playFlag = false;
                    break;
                }
            }
            break;
        }
    }

    if (allMetas.isEmpty()) {
        qCWarning(dmMusic) << "No tracks found for artist:" << artist;
        return;
    }

    qCDebug(dmMusic) << "Loading" << allMetas.size() << "tracks from artist";
    m_data->m_playerEngine->clearPlayList(playFlag);
    m_data->m_playerEngine->addMetasToPlayList(allMetas);
    
    if (!metaHash.isEmpty() && m_data->m_playerEngine->getMediaMeta().hash != metaHash) {
        qCDebug(dmMusic) << "Setting start track to:" << metaHash;
        m_data->m_playerEngine->setMediaMeta(metaHash);
    }
    
    m_data->m_playerEngine->setCurrentPlayList("artist");
    m_data->m_playerEngine->play();
    m_data->m_dataManager->setCurrentPlayliHash("artist");
    m_data->m_dataManager->clearPlayList("play", false);
    m_data->m_dataManager->addMetasToPlayList(allMetas, "play", false);
    
    qCInfo(dmMusic) << "Artist playback started successfully";
}

void Presenter::playPlaylist(const QString &playlistHash, const QString &metaHash)
{
    qCDebug(dmMusic) << "Playing playlist:" << playlistHash << "Starting with track:" << metaHash;
    if (playlistHash.isEmpty()) {
        qCWarning(dmMusic) << "Cannot play empty playlist hash";
        return;
    }

    qCInfo(dmMusic) << "Playing playlist:" << playlistHash << "Starting with track:" << metaHash;
    bool playFlag = m_data->m_playerEngine->getMediaMeta().hash != metaHash;
    QList<DMusic::MediaMeta> allMetas;
    
    if (playlistHash != "album" && playlistHash != "artist") {
        allMetas = m_data->m_dataManager->getPlaylistMetas(playlistHash);
    } else if (playlistHash == "album") {
        auto albums = m_data->m_dataManager->allAlbumInfos();
        for (auto album : albums) {
            allMetas += album.musicinfos.values();
        }
    } else {
        auto artists = m_data->m_dataManager->allArtistInfos();
        for (auto artist : artists) {
            allMetas += artist.musicinfos.values();
        }
    }
    
    if (playlistHash == "cdarole") {
        qCDebug(dmMusic) << "Adding CD audio tracks to playlist";
        allMetas = m_data->m_playerEngine->getCdaMetaInfo() + allMetas;
    }
    
    for (auto meta : allMetas) {
        if (m_data->m_playerEngine->getMediaMeta().hash == meta.hash) {
            playFlag = false;
            break;
        }
    }

    if (allMetas.isEmpty()) {
        qCWarning(dmMusic) << "No tracks found in playlist:" << playlistHash;
        return;
    }

    qCDebug(dmMusic) << "Loading" << allMetas.size() << "tracks from playlist";
    m_data->m_playerEngine->clearPlayList(playFlag);
    m_data->m_playerEngine->addMetasToPlayList(allMetas);
    
    if (!metaHash.isEmpty() && m_data->m_playerEngine->getMediaMeta().hash != metaHash) {
        qCDebug(dmMusic) << "Setting start track to:" << metaHash;
        m_data->m_playerEngine->setMediaMeta(metaHash);
    }
    
    m_data->m_playerEngine->setCurrentPlayList(playlistHash);
    if (playlistHash == "musicResult" && metaHash.isEmpty()) {
        m_data->m_playerEngine->forcePlay();
    } else {
        m_data->m_playerEngine->play();
    }

    m_data->m_dataManager->setCurrentPlayliHash(playlistHash);
    m_data->m_dataManager->clearPlayList("play", false);
    m_data->m_dataManager->addMetasToPlayList(allMetas, "play", false);
    
    qCInfo(dmMusic) << "Playlist playback started successfully";
}

void Presenter::setCurrentPlayList(const QString &playlistHash)
{
    qCDebug(dmMusic) << "Setting current playlist to:" << playlistHash;
    m_data->m_playerEngine->setCurrentPlayList(playlistHash);
    m_data->m_dataManager->setCurrentPlayliHash(playlistHash);
}

QString Presenter::getCurrentPlayList()
{
    qCDebug(dmMusic) << "Getting current playlist";
    return m_data->m_playerEngine->getCurrentPlayList();
}

void Presenter::importMetas(const QStringList &urls, const QString &playlistHash, const bool &playFalg)
{
    qCDebug(dmMusic) << "Importing metas from URLs:" << urls.size() << "Playlist:" << playlistHash << "Play flag:" << playFalg;
    if (urls.isEmpty()) {
        qInfo() << "importMetas urls is empty";
        return;
    }
    m_data->m_dataManager->importMetas(urls, playlistHash, playFalg);
}

void Presenter::addMetasToPlayList(const QStringList &metaHash, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Adding metas to playlist:" << metaHash.size() << "Playlist:" << playlistHash;
    m_data->m_dataManager->addMetasToPlayList(metaHash, playlistHash);
}

void Presenter::addAlbumToPlayList(const QString &album, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Adding album to playlist:" << album << "Playlist:" << playlistHash;
    QList<DMusic::AlbumInfo> albums = m_data->m_dataManager->allAlbumInfos();
    QList<DMusic::MediaMeta> allMetas;
    for (const DMusic::AlbumInfo &curAlbum : albums) {
        if (curAlbum.name == album) {
            allMetas += curAlbum.musicinfos.values();
            break;
        }
    }
    if (!allMetas.isEmpty()) {
        qCDebug(dmMusic) << "Adding" << allMetas.size() << "tracks from album to playlist";
        m_data->m_dataManager->addMetasToPlayList(allMetas, playlistHash);
    }
}

void Presenter::addArtistToPlayList(const QString &artist, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Adding artist to playlist:" << artist << "Playlist:" << playlistHash;

    QList<DMusic::ArtistInfo> artists = m_data->m_dataManager->allArtistInfos();
    QList<DMusic::MediaMeta> allMetas;
    for (const DMusic::ArtistInfo &curArtist : artists) {
        if (curArtist.name == artist) {
            allMetas += curArtist.musicinfos.values();
            break;
        }
    }
    if (!allMetas.isEmpty()) {
        qCDebug(dmMusic) << "Adding" << allMetas.size() << "tracks from artist to playlist";
        m_data->m_dataManager->addMetasToPlayList(allMetas, playlistHash);
    }
}

void Presenter::clearPlayList(const QString &playlistHash)
{
    qCDebug(dmMusic) << "Clearing playlist:" << playlistHash;
    // 清除播放
    if (playlistHash == "play" || playlistHash == "all") {
        qCDebug(dmMusic) << "Clearing playlist tracks";
        m_data->m_playerEngine->clearPlayList();
    }
    m_data->m_dataManager->clearPlayList(playlistHash);
}

void Presenter::removeFromPlayList(const QStringList listToDel, const QString &playlistHash, bool delFlag)
{
    qCDebug(dmMusic) << "Removing metas from playlist:" << listToDel.size() << "Playlist:" << playlistHash << "Del flag:" << delFlag;
    if (playlistHash == "play" || playlistHash == "all" || delFlag) {
        qCDebug(dmMusic) << "Removing metas from playlist tracks";
        m_data->m_playerEngine->removeMetasFromPlayList(listToDel);
    }

    m_data->m_dataManager->removeFromPlayList(listToDel, playlistHash, delFlag);

    if (m_data->m_playerEngine->getCurrentPlayList() == playlistHash
            && m_data->m_dataManager->playlistFromHash(playlistHash).sortMetas.isEmpty()) {
        qCDebug(dmMusic) << "Playlist is empty, stopping playback";
        m_data->m_playerEngine->stop();
        setCurrentPlayList("");
    }
}

void Presenter::moveMetasPlayList(const QStringList &metaHashs, const QString &playlistHash, const QString &nextHash)
{
    qCDebug(dmMusic) << "Moving metas from playlist:" << metaHashs.size() << "Playlist:" << playlistHash << "Next playlist:" << nextHash;
    if (m_data->m_dataManager->moveMetasPlayList(metaHashs, playlistHash, nextHash)
            && (playlistHash.isEmpty() || playlistHash == "play")) {
        qCDebug(dmMusic) << "Moving metas from playlist tracks";
        m_data->m_playerEngine->clearPlayList(false);
        QList<DMusic::MediaMeta> allMetas = m_data->m_dataManager->getPlaylistMetas("play");
        m_data->m_playerEngine->addMetasToPlayList(allMetas);
    }
}

QVariantList Presenter::getPlaylistMetas(const QString &hash, int count)
{
    qCDebug(dmMusic) << "Getting playlist metas:" << hash << "Count:" << count;
    QList<DMusic::MediaMeta> allMetas;
    if (hash != "cdarole") {
        qCDebug(dmMusic) << "Getting playlist metas from data manager";
        allMetas = m_data->m_dataManager->getPlaylistMetas(hash, count);
        if (hash == "play" && m_data->m_playerEngine->getCurrentPlayList() == "cdarole") {
            qCDebug(dmMusic) << "Getting playlist metas from player engine";
            allMetas = m_data->m_playerEngine->getCdaMetaInfo();
            allMetas += m_data->m_dataManager->getPlaylistMetas(hash);
        } else if (hash != "album" && hash != "artist") {
            qCDebug(dmMusic) << "Getting playlist metas from data manager";
            allMetas = m_data->m_dataManager->getPlaylistMetas(hash);
        } else if (hash == "album") {
            qCDebug(dmMusic) << "Getting playlist metas from data manager";
            auto albums = m_data->m_dataManager->allAlbumInfos();
            for (auto album : albums) {
                allMetas += album.musicinfos.values();
            }
        } else {
            qCDebug(dmMusic) << "Getting playlist metas from data manager";
            auto artists = m_data->m_dataManager->allArtistInfos();
            for (auto artist : artists) {
                allMetas += artist.musicinfos.values();
            }
        }
    } else {
        qCDebug(dmMusic) << "Getting playlist metas from player engine";
        allMetas = m_data->m_playerEngine->getCdaMetaInfo();
    }

    QVariantList allMetaList;
    for (DMusic::MediaMeta meta : allMetas) {
        allMetaList.append(Utils::metaToVariantMap(meta));
    }

    qCDebug(dmMusic) << "Returning" << allMetaList.size() << "tracks";
    return allMetaList;
}

bool Presenter::isExistMeta()
{
    qCDebug(dmMusic) << "Checking if meta exists";
    return m_data->m_dataManager->isExistMeta();
}

QVariantMap Presenter::musicInforFromHash(const QString &hash)
{
    qCDebug(dmMusic) << "Getting meta information from data manager";
    return Utils::metaToVariantMap(m_data->m_dataManager->metaFromHash(hash));
}

QVariantMap Presenter::addPlayList(const QString &name)
{
    qCDebug(dmMusic) << "Adding playlist:" << name;
    auto playlist = m_data->m_dataManager->addPlayList(name);
    
    // 创建歌单后保存数据库
    qCInfo(dmMusic) << "Saving database after creating playlist:" << name;
    saveDataToDB();
    
    emit addedPlaylist(playlist.uuid);
    return Utils::playlistToVariantMap(playlist);
}

QVariant Presenter::playlistSortType(const QString &hash)
{
    qCDebug(dmMusic) << "Getting playlist sort type:" << hash;
    return  QVariant(Utils::simplifyPlaylistSortType(m_data->m_dataManager->playlistFromHash(hash).sortType));
}

void Presenter::sortPlaylist(const int &type, const QString &hash)
{
    qCDebug(dmMusic) << "Sorting playlist:" << hash << "Type:" << type;
    m_data->m_dataManager->sortPlaylist(type, hash);
}

int Presenter::playlistMetaCount(const QString &hash)
{
    qCDebug(dmMusic) << "Getting playlist meta count:" << hash;
    return m_data->m_dataManager->playlistFromHash(hash).sortMetas.size();
}

bool Presenter::deletePlaylist(const QString &hash)
{
    qCDebug(dmMusic) << "Deleting playlist:" << hash;
    bool delFlag = false;
    if (m_data->m_dataManager->deletePlaylist(hash)) {
        qCDebug(dmMusic) << "Playlist deleted successfully";

        if (m_data->m_playerEngine->getCurrentPlayList() == hash) {
            qCDebug(dmMusic) << "Playlist is current playing, setting new current playlist";
            m_data->m_playerEngine->setCurrentPlayList("");
            m_data->m_playerEngine->stop();
        }
        emit deletedPlaylist(hash);
        delFlag = true;
    }
    qCDebug(dmMusic) << "Playlist delete result:" << delFlag;
    return delFlag;
}

bool Presenter::renamePlaylist(const QString &name, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Renaming playlist:" << playlistHash << "New name:" << name;
    bool drenameFlag = false;
    if (!name.isEmpty() && m_data->m_dataManager->renamePlaylist(name, playlistHash)) {
        qCDebug(dmMusic) << "Playlist renamed successfully";
        emit renamedPlaylist(name, playlistHash);
        drenameFlag = true;
    }
    qCDebug(dmMusic) << "Playlist rename result:" << drenameFlag;
    return drenameFlag;
}

void Presenter::movePlaylist(const QString &hash, const QString &nextHash)
{
    qCDebug(dmMusic) << "Moving playlist:" << hash << "To position:" << nextHash;
    m_data->m_dataManager->movePlaylist(hash, nextHash);
}

QVariantMap Presenter::playlistInfoFromHash(const QString &hash)
{
    qCDebug(dmMusic) << "Getting playlist information:" << hash;
    return Utils::playlistToVariantMap(m_data->m_dataManager->playlistFromHash(hash));
}

bool Presenter::isExistMeta(const QString &metaHash, const QString &playlistHash)
{
    qCDebug(dmMusic) << "Checking if meta exists:" << metaHash << "In playlist:" << playlistHash;
    return m_data->m_dataManager->isExistMeta(metaHash, playlistHash);
}

QVariantList Presenter::allPlaylistInfos()
{
    qCDebug(dmMusic) << "Getting all playlist information";
    return m_data->m_dataManager->allPlaylistVariantList();
}

QVariantList Presenter::customPlaylistInfos()
{
    qCDebug(dmMusic) << "Getting custom playlist information";
    return m_data->m_dataManager->customPlaylistVariantList();
}

QVariantList Presenter::allAlbumInfos()
{
    qCDebug(dmMusic) << "Getting all album information";
    return  m_data->m_dataManager->allAlbumVariantList();
}

QVariantList Presenter::allArtistInfos()
{
    qCDebug(dmMusic) << "Getting all artist information";
    return m_data->m_dataManager->allArtistVariantList();
}

QVariantMap Presenter::quickSearchText(const QString &text)
{
    qCDebug(dmMusic) << "Quick searching text:" << text;
    QVariantMap allDatas;
    if (text.isEmpty()) {
        qCDebug(dmMusic) << "Text is empty, returning empty data";
        return allDatas;
    }

    QStringList metaTitles;
    QList<QPair<QString, QString> > albums, artists;
    m_data->m_dataManager->quickSearchText(text, metaTitles, albums, artists);
    allDatas.insert("metas", metaTitles);

    QVariantList albumList;
    for (auto album : albums) {
        QVariantMap albumData;
        albumData.insert("name", album.first);
        albumData.insert("coverUrl", album.second);
        albumList.append(albumData);
    }
    allDatas.insert("albums", albumList);

    QVariantList artistList;
    for (auto artist : artists) {
        QVariantMap artistData;
        artistData.insert("name", artist.first);
        artistData.insert("coverUrl", artist.second);
        artistList.append(artistData);
    }
    allDatas.insert("artists", artistList);

    qCDebug(dmMusic) << "Quick search result size:" << allDatas.size();
    return allDatas;
}

QVariantMap Presenter::searchText(const QString &text, const QString &type)
{
    qCDebug(dmMusic) << "Searching text:" << text << "Type:" << type;

    QVariantMap allDatas;
    if (text.isEmpty()) {
        qCDebug(dmMusic) << "Text is empty, returning empty data";
        return allDatas;
    }

    QList<DMusic::MediaMeta> metas;
    QList<DMusic::AlbumInfo> albums;
    QList<DMusic::ArtistInfo> artists;
    m_data->m_dataManager->searchText(text, metas, albums, artists, type);

    QVariantList metaList;
    for (const DMusic::MediaMeta &meta : metas) {
        metaList.append(Utils::metaToVariantMap(meta));
    }
    allDatas.insert("metas", metaList);

    QVariantList albumList;
    for (const DMusic::AlbumInfo &album : albums) {
        albumList.append(Utils::albumToVariantMap(album));
    }
    allDatas.insert("albums", albumList);

    QVariantList artistList;
    for (const DMusic::ArtistInfo &artist : artists) {
        artistList.append(Utils::artistToVariantMap(artist));
    }
    allDatas.insert("artists", artistList);

    qCDebug(dmMusic) << "Search result size:" << allDatas.size();
    return allDatas;
}

QVariantList Presenter::searchedAlbumInfos()
{
    qCDebug(dmMusic) << "Getting searched album information";
    return m_data->m_dataManager->searchedAlbumVariantList();
}

QVariantList Presenter::searchedArtistInfos()
{
    qCDebug(dmMusic) << "Getting searched artist information";
    return m_data->m_dataManager->searchedArtistVariantList();
}

void Presenter::saveDataToDB()
{
    qCInfo(dmMusic) << "Saving application data to database";
    if (m_data->m_dataManager->valueFromSettings("base.play.remember_progress").toBool()) {
        qCDebug(dmMusic) << "Saving last playback position:" << m_data->m_playerEngine->time();
        m_data->m_dataManager->setValueToSettings("base.play.last_position", m_data->m_playerEngine->time());
    }
    m_data->m_dataManager->setValueToSettings("base.play.volume", m_data->m_playerEngine->getVolume());
    m_data->m_dataManager->setValueToSettings("base.play.mute", m_data->m_playerEngine->getMute());
    m_data->m_dataManager->setValueToSettings("base.play.media_count", m_data->m_dataManager->getPlaylistMetas("play").size());
    m_data->m_dataManager->saveDataToDB();
    qCInfo(dmMusic) << "Application data saved successfully";
}

void Presenter::syncToSettings()
{
    qCInfo(dmMusic) << "Synchronizing settings to storage";
    m_data->m_dataManager->syncToSettings();
}

void Presenter::resetToSettings()
{
    qCInfo(dmMusic) << "Resetting settings to default values";
    
    // Save current values that should be preserved
    qCDebug(dmMusic) << "Preserving critical settings before reset";
    QVariant curAskCloseAction = m_data->m_dataManager->valueFromSettings("base.close.is_close");
    QVariant curLastPlaylist = m_data->m_dataManager->valueFromSettings("base.play.last_playlist");
    QVariant curLastMeta = m_data->m_dataManager->valueFromSettings("base.play.last_meta");
    QVariant curLastPosition = m_data->m_dataManager->valueFromSettings("base.play.last_position");

    // Save equalizer settings
    qCDebug(dmMusic) << "Preserving equalizer settings";
    QVariant curEqualizerBaud_12K = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_12K");
    QVariant curEqualizerBaud_14K = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_14K");
    QVariant curEqualizerBaud_16K = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_16K");
    QVariant curEqualizerBaud_170 = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_170");
    QVariant curEqualizerBaud_1K = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_1K");
    QVariant curEqualizerBaud_310 = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_310");
    QVariant curEqualizerBaud_3K = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_3K");
    QVariant curEqualizerBaud_60 = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_60");
    QVariant curEqualizerBaud_600 = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_600");
    QVariant curEqualizerBaud_6K = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_6K");
    QVariant curEqualizerBaud_pre = m_data->m_dataManager->valueFromSettings("equalizer.all.baud_pre");
    QVariant curEqualizerCurEffect = m_data->m_dataManager->valueFromSettings("equalizer.all.curEffect");
    QVariant curEqualizerSwitch = m_data->m_dataManager->valueFromSettings("equalizer.all.switch");

    qCDebug(dmMusic) << "Performing settings reset";
    m_data->m_dataManager->resetToSettings();

    // Restore preserved settings
    qCDebug(dmMusic) << "Restoring preserved settings";
    m_data->m_dataManager->setValueToSettings("base.close.is_close", curAskCloseAction);
    m_data->m_dataManager->setValueToSettings("base.play.last_playlist", curLastPlaylist);
    m_data->m_dataManager->setValueToSettings("base.play.last_meta", curLastMeta);
    m_data->m_dataManager->setValueToSettings("base.play.last_position", curLastPosition);

    // Restore equalizer settings
    qCDebug(dmMusic) << "Restoring equalizer settings";
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_12K", curEqualizerBaud_12K);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_14K", curEqualizerBaud_14K);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_16K", curEqualizerBaud_16K);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_170", curEqualizerBaud_170);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_1K", curEqualizerBaud_1K);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_310", curEqualizerBaud_310);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_3K", curEqualizerBaud_3K);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_60", curEqualizerBaud_60);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_600", curEqualizerBaud_600);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_6K", curEqualizerBaud_6K);
    m_data->m_dataManager->setValueToSettings("equalizer.all.baud_pre", curEqualizerBaud_pre);
    m_data->m_dataManager->setValueToSettings("equalizer.all.curEffect", curEqualizerCurEffect);
    m_data->m_dataManager->setValueToSettings("equalizer.all.switch", curEqualizerSwitch);

    emit resetedFromSettings();
    qCInfo(dmMusic) << "Settings reset completed successfully";
}

QVariant Presenter::valueFromSettings(const QString &key)
{
    qCDebug(dmMusic) << "Getting value for key:" << key;
    return m_data->m_dataManager->valueFromSettings(key);
}

void Presenter::setValueToSettings(const QString &key, const QVariant &value)
{
    qCDebug(dmMusic) << "Setting value for key:" << key << "Value:" << value;
    if (value.isNull()) {
        qCWarning(dmMusic) << "Attempted to set null value for key:" << key;
        return;
    }
    
    qCDebug(dmMusic) << "Setting value for key:" << key << "Value:" << value;
    m_data->m_dataManager->setValueToSettings(key, value);
    
    if (key == "base.play.fade_in_out") {
        qCDebug(dmMusic) << "Updating fade in/out setting:" << value.toBool();
        m_data->m_playerEngine->setFadeInOut(value.toBool());
    }
    
    qCDebug(dmMusic) << "Value set successfully for key:" << key;
    emit valueChangedFromSettings(key, value);
}

QStringList Presenter::detectEncodings(const QString &metaHash)
{
    qCDebug(dmMusic) << "Detecting encodings for meta:" << metaHash;
    DMusic::MediaMeta meta = m_data->m_dataManager->metaFromHash(metaHash);
    return m_data->m_audioAnalysis->detectEncodings(meta);
}

void Presenter::updateMetaCodec(const QString &metaHash, const QString &codecStr)
{
    qCInfo(dmMusic) << "Updating meta codec - Hash:" << metaHash << "Codec:" << codecStr;
    DMusic::MediaMeta meta = m_data->m_dataManager->metaFromHash(metaHash);
    m_data->m_audioAnalysis->convertMetaCodec(meta, codecStr);
    m_data->m_dataManager->updateMetaCodec(meta);
}

void Presenter::play()
{
    qCDebug(dmMusic) << "Playing media";
    if (m_data->m_playerEngine->isEmpty()) {
        qCDebug(dmMusic) << "Playlist is empty, playing all";
        playPlaylist("all");
    } else {
        qCDebug(dmMusic) << "Playing current media";
        m_data->m_playerEngine->play();
    }
}

void Presenter::playPre()
{
    qCDebug(dmMusic) << "Playing previous media";
    m_data->m_playerEngine->playPreMeta();
}

void Presenter::playNext()
{
    qCDebug(dmMusic) << "Playing next media";
    m_data->m_playerEngine->playNextMeta(false);
}

void Presenter::pause()
{
    qCDebug(dmMusic) << "Pausing media";
    m_data->m_playerEngine->pause();
}

void Presenter::playPause()
{
    qCDebug(dmMusic) << "Playing or pausing media";
    if (m_data->m_playerEngine->isEmpty()) {
        qCDebug(dmMusic) << "Playlist is empty, playing all";
        playPlaylist("all");
    } else {
        qCDebug(dmMusic) << "Playing or pausing current media";
        m_data->m_playerEngine->playPause();
    }
}

void Presenter::resume()
{
    qCDebug(dmMusic) << "Resuming media";
    if (m_data->m_playerEngine->getMediaMeta().localPath.isEmpty()) {
        qCDebug(dmMusic) << "Media local path is empty, forcing play";
        m_data->m_playerEngine->forcePlay();
    } else {
        qCDebug(dmMusic) << "Resuming current media";
        m_data->m_playerEngine->play();
    }
}

void Presenter::stop()
{
    qCDebug(dmMusic) << "Stopping media";
    m_data->m_playerEngine->stop();
}


