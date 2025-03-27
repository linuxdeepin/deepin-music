// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "presenter.h"

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
        m_playerEngine = new PlayerEngine(m_parent);
        m_dataManager = new DataManager(m_playerEngine->supportedSuffixList(), m_parent);
        m_playerEngine->setPlaybackMode((DmGlobal::PlaybackMode)m_dataManager->valueFromSettings("base.play.playmode").toInt());
        m_pkmeans = new CKMeans;
        m_playerEngine->addMetasToPlayList(m_dataManager->getPlaylistMetas("play"));
        m_audioAnalysis = new AudioAnalysis(m_parent);
    }
    ~PresenterPrivate()
    {
        m_audioAnalysis->stopRecorder();
        // 颜色聚类
        if (m_pkmeans) {
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
    qRegisterMetaType<DMusic::MediaMeta>("DMusic::MediaMeta");
    qRegisterMetaType<DMusic::PlaylistInfo>("DMusic::PlaylistInfo");
    qRegisterMetaType<DMusic::AlbumInfo>("DMusic::AlbumInfo");
    qRegisterMetaType<DMusic::ArtistInfo>("DMusic::ArtistInfo");
    DmGlobal::initPath();
    DmGlobal::setUnknownAlbumText(unknownAlbumStr);
    DmGlobal::setUnknownArtistText(unknownArtistStr);

    m_data = new PresenterPrivate(this);
    connect(m_data->m_playerEngine, &PlayerEngine::volumeChanged, this, &Presenter::volumeChanged);
    connect(m_data->m_playerEngine, &PlayerEngine::muteChanged, this, &Presenter::muteChanged);
    connect(m_data->m_playerEngine, &PlayerEngine::metaChanged, this, [ = ]() {
        m_data->m_audioAnalysis->parseAudioBuffer(m_data->m_playerEngine->getMediaMeta());
        m_data->m_dataManager->setValueToSettings("base.play.last_meta", m_data->m_playerEngine->getMediaMeta().hash, true);
        emit metaChanged();
    });
    connect(m_data->m_playerEngine, &PlayerEngine::playPictureChanged, this, &Presenter::updatePlayingIcon);
    connect(m_data->m_playerEngine, &PlayerEngine::positionChanged, this, &Presenter::positionChanged);
    connect(m_data->m_playerEngine, &PlayerEngine::playbackStatusChanged, this, [ = ](DmGlobal::PlaybackStatus status) {
        if (status == DmGlobal::Playing) {
            m_data->m_audioAnalysis->startRecorder();
        } else {
            m_data->m_audioAnalysis->suspendRecorder();
        }
        emit playbackStatusChanged(status);
    });
    connect(m_data->m_playerEngine, &PlayerEngine::sendCdaStatus, this, [ = ](int state) {
        if (state == -1) {
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
            emit updateCDStatus(state);
        }
    });
    connect(m_data->m_playerEngine, &PlayerEngine::playPlaylistRequested, this, [this](const QString &playlistHash){
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
        if (playlistHashs.contains("play") && addToPlay) m_data->m_playerEngine->addMetasToPlayList(QList<DMusic::MediaMeta>() << meta);
        emit addOneMeta(playlistHashs, Utils::metaToVariantMap(meta));
    });
    connect(m_data->m_dataManager, &DataManager::signalImportFinished, this, [ = ](QStringList playlistHashs, int failCount, int sucessCount, int existCount, QString mediaHash) {
        // 自动播放
        if (!mediaHash.isEmpty()) {
            if (m_data->m_playerEngine->getMediaMeta().hash != mediaHash)
                m_data->m_playerEngine->setMediaMeta(mediaHash);
            m_data->m_playerEngine->play();
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
            //载入当前设置音效
            auto curIndex = m_data->m_dataManager->valueFromSettings("equalizer.all.curEffect").toInt();
            //非自定义模式时
            if (curIndex > 0) {
                m_data->m_playerEngine->loadFromPreset(uint(curIndex - 1));
                //设置放大值
                m_data->m_playerEngine->setPreamplification(m_data->m_playerEngine->preamplification());
                for (int i = 0 ; i < 10; i++) {
                    //设置频率值
                    m_data->m_playerEngine->setAmplificationForBandAt(m_data->m_playerEngine->amplificationForBandAt(uint(i)), uint(i));
                }
            } else {
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
}

Presenter::~Presenter()
{
    if (m_data) {
        delete m_data;
        m_data = nullptr;
    }
}

void Presenter::setMprisPlayer(const QString &serviceName, const QString &desktopEntry, const QString &identity)
{
    m_data->m_playerEngine->setMprisPlayer(serviceName, desktopEntry, identity);
}

QStringList Presenter::supportedSuffixList() const
{
    QStringList suffixList;
    for (QString str : m_data->m_playerEngine->supportedSuffixList()) {
        suffixList.append("*." + str);
    }
    return suffixList;
}

QColor Presenter::getMainColorByKmeans()
{
    return m_data->m_pkmeans->getCommColorMain();
}

QColor Presenter::getSecondColorByKmeans()
{
    return m_data->m_pkmeans->getCommColorSecond();
}

QImage Presenter::getEffectImage()
{
    return m_data->m_pkmeans->getShowImage();
}

void Presenter::setEffectImage(const QImage &img)
{
    m_data->m_pkmeans->setShowImage(img);
}

void Presenter::forceExit()
{
    saveDataToDB();
    qApp->processEvents();
    QCoreApplication::exit(0);
    _Exit(0);
}

QVariantList Presenter::getLyrics()
{
    QVariantList lyrics;
    DMusic::MediaMeta meta = m_data->m_playerEngine->getMediaMeta();
    if (!meta.localPath.isEmpty()) {
        QFileInfo fileInfo(meta.localPath);
        QString lrcPath = fileInfo.dir().path() + QDir::separator() + fileInfo.completeBaseName() + ".lrc";
        // 同目录下歌词文件不存在，读取缓存中解析的歌词
        QFile file(lrcPath);
        if (!file.exists()) {
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
    return lyrics;
}

void Presenter::setActivateMeta(const QString &metaHash)
{
    qDebug() << __func__;
    m_data->m_playerEngine->setMediaMeta(metaHash);
}

QImage Presenter::getActivateMetImage()
{
    qDebug() << __func__;
    return AudioAnalysis::getMetaCoverImage(m_data->m_playerEngine->getMediaMeta());
}

QVariantMap Presenter::getActivateMeta()
{
    qDebug() << __func__;
    return Utils::metaToVariantMap(m_data->m_playerEngine->getMediaMeta());
}

QVariant Presenter::getPlaybackStatus()
{
    qDebug() << __func__;
    return QVariant(m_data->m_playerEngine->playbackStatus());
}

void Presenter::setMute(bool mute)
{
    qDebug() << __func__;
    m_data->m_playerEngine->setMute(mute);
}

bool Presenter::getMute()
{
    qDebug() << __func__;
    return m_data->m_playerEngine->getMute();
}

void Presenter::setVolume(int volume)
{
    qDebug() << __func__;
    m_data->m_playerEngine->setVolume(volume);
}

int Presenter::getVolume()
{
    qDebug() << __func__;
    return m_data->m_playerEngine->getVolume();
}

void Presenter::setPlaybackMode(QVariant mode)
{
    qDebug() << __func__;
    m_data->m_playerEngine->setPlaybackMode(mode.value<DmGlobal::PlaybackMode>());
    m_data->m_dataManager->setValueToSettings("base.play.playmode", mode.value<DmGlobal::PlaybackMode>());
}

QVariant Presenter::getPlaybackMode()
{
    qDebug() << __func__;
    return QVariant(m_data->m_playerEngine->getPlaybackMode());
}

void Presenter::setPosition(qint64 position)
{
    qDebug() << __func__;
    m_data->m_playerEngine->setTime(position);
}

qint64 Presenter::getPosition()
{
    qDebug() << __func__;
    return m_data->m_playerEngine->time();
}

void Presenter::setEQ(bool enabled, int curIndex, QVariantList indexbaud)
{
    if (enabled) {
        //非自定义模式时
        if (curIndex > 0) {
            m_data->m_playerEngine->loadFromPreset(uint(curIndex - 1));
            //设置放大值
            m_data->m_playerEngine->setPreamplification(m_data->m_playerEngine->preamplification());
            for (int i = 0 ; i < 10; i++) {
                //设置频率值
                m_data->m_playerEngine->setAmplificationForBandAt(m_data->m_playerEngine->amplificationForBandAt(uint(i)), uint(i));
            }
        } else {
            if (indexbaud.size() == 0) {
                return;
            } else {
                m_data->m_playerEngine->setPreamplification(indexbaud.at(0).toInt());
                for (int i = 1; i < 11; i++) {
                    m_data->m_playerEngine->setAmplificationForBandAt(indexbaud.at(i).toInt(), uint(i - 1));
                }
            }
        }
    }
}

void Presenter::setEQEnable(bool enable)
{
    m_data->m_playerEngine->setEqualizerEnabled(enable);
}

void Presenter::setEQpre(int val)
{
    m_data->m_playerEngine->setPreamplification(val);
}

void Presenter::setEQbauds(int index, int val)
{
    m_data->m_playerEngine->setAmplificationForBandAt(uint(val), uint(index));
}

void Presenter::setEQCurMode(int curIndex)
{
    //非自定义模式时
    if (curIndex != 0) {
        m_data->m_playerEngine->loadFromPreset(uint(curIndex - 1));
        //设置放大值
        m_data->m_playerEngine->setPreamplification(m_data->m_playerEngine->preamplification());
        for (int i = 0 ; i < 10; i++) {
            //设置频率值
            m_data->m_playerEngine->setAmplificationForBandAt(m_data->m_playerEngine->amplificationForBandAt(uint(i)), uint(i));
        }
    }
}

void Presenter::showMetaFile(const QString &hash)
{
    DMusic::MediaMeta meta = m_data->m_dataManager->metaFromHash(hash);
    if (meta.localPath.isEmpty()) return;

    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                             QStringLiteral("/org/freedesktop/FileManager1"),
                             QStringLiteral("org.freedesktop.FileManager1"));
    interface.call("ShowItems", QStringList() << meta.localPath, QString());
}

bool Presenter::nextMetaFromPlay(const QString &metaHash)
{
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
    return flag;
}

bool Presenter::preMetaFromPlay(const QString &metaHash)
{
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
    return flag;
}

void Presenter::playAlbum(const QString &album, const QString &metaHash)
{
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

    m_data->m_playerEngine->clearPlayList(playFlag);
    m_data->m_playerEngine->addMetasToPlayList(allMetas);
    if (!metaHash.isEmpty() && m_data->m_playerEngine->getMediaMeta().hash != metaHash) {
        m_data->m_playerEngine->setMediaMeta(metaHash);
    }
    m_data->m_playerEngine->setCurrentPlayList("album");
    m_data->m_playerEngine->play();
    m_data->m_dataManager->setCurrentPlayliHash("album");
    m_data->m_dataManager->clearPlayList("play", false);
    m_data->m_dataManager->addMetasToPlayList(allMetas, "play", false);
}

void Presenter::playArtist(const QString &artist, const QString &metaHash)
{
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

    m_data->m_playerEngine->clearPlayList(playFlag);
    m_data->m_playerEngine->addMetasToPlayList(allMetas);
    if (!metaHash.isEmpty() && m_data->m_playerEngine->getMediaMeta().hash != metaHash) {
        m_data->m_playerEngine->setMediaMeta(metaHash);
    }
    m_data->m_playerEngine->setCurrentPlayList("artist");
    m_data->m_playerEngine->play();
    m_data->m_dataManager->setCurrentPlayliHash("artist");
    m_data->m_dataManager->clearPlayList("play", false);
    m_data->m_dataManager->addMetasToPlayList(allMetas, "play", false);
}

void Presenter::playPlaylist(const QString &playlistHash, const QString &metaHash)
{
    if (playlistHash.isEmpty()) return;

    bool playFlag = m_data->m_playerEngine->getMediaMeta().hash != metaHash;
    QList<DMusic::MediaMeta> allMetas = m_data->m_dataManager->getPlaylistMetas(playlistHash);
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
        allMetas = m_data->m_playerEngine->getCdaMetaInfo() + allMetas;
    }
    for (auto meta : allMetas) {
        if (m_data->m_playerEngine->getMediaMeta().hash == meta.hash) {
            playFlag = false;
            break;
        }
    }

    m_data->m_playerEngine->clearPlayList(playFlag);
    m_data->m_playerEngine->addMetasToPlayList(allMetas);
    if (!metaHash.isEmpty() && m_data->m_playerEngine->getMediaMeta().hash != metaHash) {
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
}

void Presenter::setCurrentPlayList(const QString &playlistHash)
{
    qDebug() << __func__;
    m_data->m_playerEngine->setCurrentPlayList(playlistHash);
    m_data->m_dataManager->setCurrentPlayliHash(playlistHash);
}

QString Presenter::getCurrentPlayList()
{
    qDebug() << __func__;
    return m_data->m_playerEngine->getCurrentPlayList();
}

void Presenter::importMetas(const QStringList &urls, const QString &playlistHash, const bool &playFalg)
{
    qDebug() << __func__;
    if (urls.isEmpty()) {
        qInfo() << "importMetas urls is empty";
        return;
    }
    m_data->m_dataManager->importMetas(urls, playlistHash, playFalg);
}

void Presenter::addMetasToPlayList(const QStringList &metaHash, const QString &playlistHash)
{
    qDebug() << __func__;
    m_data->m_dataManager->addMetasToPlayList(metaHash, playlistHash);
}

void Presenter::addAlbumToPlayList(const QString &album, const QString &playlistHash)
{
    QList<DMusic::AlbumInfo> albums = m_data->m_dataManager->allAlbumInfos();
    QList<DMusic::MediaMeta> allMetas;
    for (const DMusic::AlbumInfo &curAlbum : albums) {
        if (curAlbum.name == album) {
            allMetas += curAlbum.musicinfos.values();
            break;
        }
    }
    if (!allMetas.isEmpty())
        m_data->m_dataManager->addMetasToPlayList(allMetas, playlistHash);
}

void Presenter::addArtistToPlayList(const QString &artist, const QString &playlistHash)
{
    QList<DMusic::ArtistInfo> artists = m_data->m_dataManager->allArtistInfos();
    QList<DMusic::MediaMeta> allMetas;
    for (const DMusic::ArtistInfo &curArtist : artists) {
        if (curArtist.name == artist) {
            allMetas += curArtist.musicinfos.values();
            break;
        }
    }
    if (!allMetas.isEmpty())
        m_data->m_dataManager->addMetasToPlayList(allMetas, playlistHash);
}

void Presenter::clearPlayList(const QString &playlistHash)
{
    qDebug() << __func__;
    // 清除播放
    if (playlistHash == "play" || playlistHash == "all")
        m_data->m_playerEngine->clearPlayList();
    m_data->m_dataManager->clearPlayList(playlistHash);
}

void Presenter::removeFromPlayList(const QStringList listToDel, const QString &playlistHash, bool delFlag)
{
    qDebug() << __func__;
    if (playlistHash == "play" || playlistHash == "all" || delFlag)
        m_data->m_playerEngine->removeMetasFromPlayList(listToDel);

    m_data->m_dataManager->removeFromPlayList(listToDel, playlistHash, delFlag);

    if (m_data->m_playerEngine->getCurrentPlayList() == playlistHash
            && m_data->m_dataManager->playlistFromHash(playlistHash).sortMetas.isEmpty()) {
        m_data->m_playerEngine->stop();
        setCurrentPlayList("");
    }
}

void Presenter::moveMetasPlayList(const QStringList &metaHashs, const QString &playlistHash, const QString &nextHash)
{
    if (m_data->m_dataManager->moveMetasPlayList(metaHashs, playlistHash, nextHash)
            && (playlistHash.isEmpty() || playlistHash == "play")) {
        m_data->m_playerEngine->clearPlayList(false);
        QList<DMusic::MediaMeta> allMetas = m_data->m_dataManager->getPlaylistMetas("play");
        m_data->m_playerEngine->addMetasToPlayList(allMetas);
    }
}

QVariantList Presenter::getPlaylistMetas(const QString &hash, int count)
{
    qDebug() << __func__;
    QList<DMusic::MediaMeta> allMetas;
    if (hash != "cdarole") {
        allMetas = m_data->m_dataManager->getPlaylistMetas(hash, count);
        if (hash == "play" && m_data->m_playerEngine->getCurrentPlayList() == "cdarole") {
            allMetas = m_data->m_playerEngine->getCdaMetaInfo();
            allMetas += m_data->m_dataManager->getPlaylistMetas(hash);
        } else if (hash != "album" && hash != "artist") {
            allMetas = m_data->m_dataManager->getPlaylistMetas(hash);
        } else if (hash == "album") {
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
    } else {
        allMetas = m_data->m_playerEngine->getCdaMetaInfo();
    }

    QVariantList allMetaList;
    for (DMusic::MediaMeta meta : allMetas) {
        allMetaList.append(Utils::metaToVariantMap(meta));
    }

    return allMetaList;
}

bool Presenter::isExistMeta()
{
    qDebug() << __func__;
    return m_data->m_dataManager->isExistMeta();
}

QVariantMap Presenter::musicInforFromHash(const QString &hash)
{
    qDebug() << __func__;
    return Utils::metaToVariantMap(m_data->m_dataManager->metaFromHash(hash));
}

QVariantMap Presenter::addPlayList(const QString &name)
{
    qDebug() << __func__;
    auto playlist = m_data->m_dataManager->addPlayList(name);
    emit addedPlaylist(playlist.uuid);
    return Utils::playlistToVariantMap(playlist);
}

QVariant Presenter::playlistSortType(const QString &hash)
{
    qDebug() << __func__;
    return  QVariant(Utils::simplifyPlaylistSortType(m_data->m_dataManager->playlistFromHash(hash).sortType));
}

void Presenter::sortPlaylist(const int &type, const QString &hash)
{
    qDebug() << __func__;
    m_data->m_dataManager->sortPlaylist(type, hash);
}

int Presenter::playlistMetaCount(const QString &hash)
{
    qDebug() << __func__;
    return m_data->m_dataManager->playlistFromHash(hash).sortMetas.size();
}

bool Presenter::deletePlaylist(const QString &hash)
{
    qDebug() << __func__;
    bool delFlag = false;
    if (m_data->m_dataManager->deletePlaylist(hash)) {
        if (m_data->m_playerEngine->getCurrentPlayList() == hash) {
            m_data->m_playerEngine->setCurrentPlayList("");
            m_data->m_playerEngine->stop();
        }
        emit deletedPlaylist(hash);
        delFlag = true;
    }
    return delFlag;
}

bool Presenter::renamePlaylist(const QString &name, const QString &playlistHash)
{
    qDebug() << __func__;
    bool drenameFlag = false;
    if (!name.isEmpty() && m_data->m_dataManager->renamePlaylist(name, playlistHash)) {
        emit renamedPlaylist(name, playlistHash);
        drenameFlag = true;
    }
    return drenameFlag;
}

void Presenter::movePlaylist(const QString &hash, const QString &nextHash)
{
    m_data->m_dataManager->movePlaylist(hash, nextHash);
}

QVariantMap Presenter::playlistInfoFromHash(const QString &hash)
{
    qDebug() << __func__;
    return Utils::playlistToVariantMap(m_data->m_dataManager->playlistFromHash(hash));
}

bool Presenter::isExistMeta(const QString &metaHash, const QString &playlistHash)
{
    qDebug() << __func__;
    return m_data->m_dataManager->isExistMeta(metaHash, playlistHash);
}

QVariantList Presenter::allPlaylistInfos()
{
    qDebug() << __func__;
    return m_data->m_dataManager->allPlaylistVariantList();
}

QVariantList Presenter::customPlaylistInfos()
{
    qDebug() << __func__;
    return m_data->m_dataManager->customPlaylistVariantList();
}

QVariantList Presenter::allAlbumInfos()
{
    qDebug() << __func__;
    return  m_data->m_dataManager->allAlbumVariantList();
}

QVariantList Presenter::allArtistInfos()
{
    qDebug() << __func__;
    return m_data->m_dataManager->allArtistVariantList();
}

QVariantMap Presenter::quickSearchText(const QString &text)
{
    QVariantMap allDatas;
    if (text.isEmpty())
        return allDatas;

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

    return allDatas;
}

QVariantMap Presenter::searchText(const QString &text, const QString &type)
{
    QVariantMap allDatas;
    if (text.isEmpty())
        return allDatas;

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

    return allDatas;
}

QVariantList Presenter::searchedAlbumInfos()
{
    qDebug() << __func__;
    return m_data->m_dataManager->searchedAlbumVariantList();
}

QVariantList Presenter::searchedArtistInfos()
{
    qDebug() << __func__;
    return m_data->m_dataManager->searchedArtistVariantList();
}

void Presenter::saveDataToDB()
{
    if (m_data->m_dataManager->valueFromSettings("base.play.remember_progress").toBool()) {
        m_data->m_dataManager->setValueToSettings("base.play.last_position", m_data->m_playerEngine->time());
    }
    m_data->m_dataManager->setValueToSettings("base.play.volume", m_data->m_playerEngine->getVolume());
    m_data->m_dataManager->setValueToSettings("base.play.mute", m_data->m_playerEngine->getMute());
    m_data->m_dataManager->setValueToSettings("base.play.media_count", m_data->m_dataManager->getPlaylistMetas("play").size());
    m_data->m_dataManager->saveDataToDB();
}

void Presenter::syncToSettings()
{
    m_data->m_dataManager->syncToSettings();
}

void Presenter::resetToSettings()
{
    QVariant curAskCloseAction = m_data->m_dataManager->valueFromSettings("base.close.is_close");
    QVariant curLastPlaylist = m_data->m_dataManager->valueFromSettings("base.play.last_playlist");
    QVariant curLastMeta = m_data->m_dataManager->valueFromSettings("base.play.last_meta");
    QVariant curLastPosition = m_data->m_dataManager->valueFromSettings("base.play.last_position");

    // 保留均衡器的配置
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

    m_data->m_dataManager->resetToSettings();

    m_data->m_dataManager->setValueToSettings("base.close.is_close", curAskCloseAction);
    m_data->m_dataManager->setValueToSettings("base.play.last_playlist", curLastPlaylist);
    m_data->m_dataManager->setValueToSettings("base.play.last_meta", curLastMeta);
    m_data->m_dataManager->setValueToSettings("base.play.last_position", curLastPosition);

    // 恢复均衡器设置
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
}

QVariant Presenter::valueFromSettings(const QString &key)
{
    return m_data->m_dataManager->valueFromSettings(key);
}

void Presenter::setValueToSettings(const QString &key, const QVariant &value)
{
    if (value.isNull()) return;
    m_data->m_dataManager->setValueToSettings(key, value);
    if (key == "base.play.fade_in_out") {
        m_data->m_playerEngine->setFadeInOut(value.toBool());
    }
    emit valueChangedFromSettings(key, value);
}

QStringList Presenter::detectEncodings(const QString &metaHash)
{
    DMusic::MediaMeta meta = m_data->m_dataManager->metaFromHash(metaHash);
    return  m_data->m_audioAnalysis->detectEncodings(meta);
}

void Presenter::updateMetaCodec(const QString &metaHash, const QString &codecStr)
{
    qDebug() << __func__;
    DMusic::MediaMeta meta = m_data->m_dataManager->metaFromHash(metaHash);
    m_data->m_audioAnalysis->convertMetaCodec(meta, codecStr);
    m_data->m_dataManager->updateMetaCodec(meta);
}

void Presenter::play()
{
    qDebug() << __func__;
    if (m_data->m_playerEngine->isEmpty()) {
        playPlaylist("all");
    } else {
        m_data->m_playerEngine->play();
    }
}

void Presenter::playPre()
{
    qDebug() << __func__;
    m_data->m_playerEngine->playPreMeta();
}

void Presenter::playNext()
{
    qDebug() << __func__;
    m_data->m_playerEngine->playNextMeta(false);
}

void Presenter::pause()
{
    qDebug() << __func__;
    m_data->m_playerEngine->pause();
}

void Presenter::playPause()
{
    qDebug() << __func__;
    if (m_data->m_playerEngine->isEmpty()) {
        playPlaylist("all");
    } else {
        m_data->m_playerEngine->playPause();
    }
}

void Presenter::resume()
{
    qDebug() << __func__;
    if (m_data->m_playerEngine->getMediaMeta().localPath.isEmpty()) {
        m_data->m_playerEngine->forcePlay();
    } else {
        m_data->m_playerEngine->play();
    }
}

void Presenter::stop()
{
    qDebug() << __func__;
    m_data->m_playerEngine->stop();
}
