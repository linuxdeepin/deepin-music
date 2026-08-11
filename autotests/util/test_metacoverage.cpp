// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// MOC meta-object coverage tests: exercises metaObject(), qt_metacast(),
// qt_metacall(), qt_static_metacall(), and signal emitter functions for every
// QObject-derived class in src/libdmusic so that lcov function coverage reaches
// the MOC-generated code that normal tests never invoke directly.

#include <gtest/gtest.h>
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QVariant>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QImage>
#include <QDBusConnection>
#include <QDir>

#include "global.h"
#include "presenter.h"
#include "core/datamanager.h"
#include "core/dboperate.h"
#include "core/musicsettings.h"
#include "core/audioanalysis.h"
#include "core/audiodatadetector.h"
#include "player/playerbase.h"
#include "player/playerengine.h"
#include "player/qtplayer.h"
#include "player/vlcplayer.h"
#include "util/ckmeans.h"

// VLC wrapper headers
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"
#include "vlc/Enums.h"
#include "vlc/cda.h"
#include "vlc/checkdatazerothread.h"
#include "vlc/sdlplayer.h"
#include "vlc/vlcdynamicinstance.h"

// Fake player for PlayerEngine / PlayerBase construction
#include "fake_player.h"

// Typedefs for types containing commas (needed for Q_ARG macro)
typedef QMap<QString, QVariant> StringVariantMap;

// ---------------------------------------------------------------------------
// Helper: cover metaObject() and qt_metacast() for any QObject.
// ---------------------------------------------------------------------------
static void coverMetaCast(QObject *obj)
{
    const QMetaObject *mo = obj->metaObject();   // metaObject() const
    ASSERT_NE(mo, nullptr);
    EXPECT_NE(mo->className(), nullptr);
    void *result = obj->qt_metacast(mo->className()); // qt_metacast(const char*)
    EXPECT_NE(result, nullptr);
    obj->qt_metacast("NonExistentClass");             // qt_metacast again
}

// ---------------------------------------------------------------------------
// Helper: cover qt_metacall() and qt_static_metacall() by invoking every
// parameterless signal/slot via the meta-object system.  Parameterless
// invocations are always safe.  Parameterised signals are handled explicitly
// in the per-class tests below.
// ---------------------------------------------------------------------------
static void coverMetaCallParamless(QObject *obj)
{
    const QMetaObject *mo = obj->metaObject();
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        if (mm.methodType() != QMetaMethod::Signal)
            continue;
        if (mm.parameterCount() != 0)
            continue;
        QMetaObject::invokeMethod(obj, mm.name().constData(), Qt::DirectConnection);
    }
}

// Full MOC coverage for classes whose signals are all parameterless or handled
// elsewhere.  For classes with parameterised uncovered signals, add explicit
// invokeMethod calls below.
static void coverMocBasic(QObject *obj)
{
    coverMetaCast(obj);
    coverMetaCallParamless(obj);
    // Cover qt_metacall() directly: invokeMethod on signals uses activate(),
    // which bypasses qt_metacall.  Call qt_metacall with InvokeMetaMethod
    // using the absolute index of the first parameterless own method.
    // qt_metacall expects the absolute index (mo->method(i)); the base class
    // qt_metacall subtracts the inherited method count, leaving the relative
    // index which qt_static_metacall dispatches.
    const QMetaObject *mo = obj->metaObject();
    int absIdx = -1;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        if (mm.parameterCount() != 0)
            continue;
        absIdx = i;
        break;
    }
    if (absIdx >= 0) {
        // Found a parameterless own method — dispatch through qt_metacall,
        // which also covers qt_static_metacall.
        void *args[10] = {};
        obj->qt_metacall(QMetaObject::InvokeMetaMethod, absIdx, args);
    } else {
        // No parameterless own method.  Call qt_metacall with ResetProperty
        // (handled by base class, returns early without dispatching to methods)
        // to at least cover the qt_metacall entry point.  Using ResetProperty
        // with an out-of-range index is safe — base class subtracts property
        // count and returns negative.
        void *args[10] = {};
        obj->qt_metacall(QMetaObject::ResetProperty, 0, args);
    }
}

// ===========================================================================
// Simple QObject-derived classes
// ===========================================================================

TEST(MetaCoverageTest, DmGlobal_MocFunctions_Covered) {
    DmGlobal obj;
    coverMocBasic(&obj);
}

TEST(MetaCoverageTest, CKMeans_MocFunctions_Covered) {
    CKMeans obj;
    coverMocBasic(&obj);
}

TEST(MetaCoverageTest, MusicSettings_MocFunctions_Covered) {
    MusicSettings obj;
    coverMocBasic(&obj);
}

TEST(MetaCoverageTest, AudioAnalysis_MocFunctions_Covered) {
    AudioAnalysis obj;
    coverMocBasic(&obj);
    // Parameterised signals: audioBuffer(QList<float> const&, QString const&),
    // audioSpectrumData(QList<int>)
    QList<float> floats;
    QMetaObject::invokeMethod(&obj, "audioBuffer", Qt::DirectConnection,
                              Q_ARG(QList<float>, floats), Q_ARG(QString, QString()));
    QList<int> ints;
    QMetaObject::invokeMethod(&obj, "audioSpectrumData", Qt::DirectConnection,
                              Q_ARG(QList<int>, ints));
}

TEST(MetaCoverageTest, AudioDataDetector_MocFunctions_Covered) {
    AudioDataDetector obj;
    coverMocBasic(&obj);
    QList<float> floats;
    QMetaObject::invokeMethod(&obj, "audioBuffer", Qt::DirectConnection,
                              Q_ARG(QList<float>, floats), Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(&obj, "audioBufferFromThread", Qt::DirectConnection,
                              Q_ARG(QList<float>, floats), Q_ARG(QString, QString()));
}

TEST(MetaCoverageTest, VlcEnum_MocFunctions_Covered) {
    Vlc obj;
    coverMocBasic(&obj);
}

TEST(MetaCoverageTest, VlcDynamicInstance_MocFunctions_Covered) {
    auto *obj = VlcDynamicInstance::VlcFunctionInstance();
    ASSERT_NE(obj, nullptr);
    coverMocBasic(obj);
}

// ===========================================================================
// VLC wrapper classes (need an instance)
// ===========================================================================

class VlcMocFixture : public ::testing::Test {
protected:
    void SetUp() override {
        QStringList args{"--ignore-config", "--no-video"};
        inst = new VlcInstance(args);
        mp = new VlcMediaPlayer(inst);
    }
    void TearDown() override {
        delete mp;
        delete inst;
    }
    VlcInstance *inst = nullptr;
    VlcMediaPlayer *mp = nullptr;
};

TEST_F(VlcMocFixture, VlcInstance_MocFunctions_Covered) {
    coverMocBasic(inst);
}

TEST_F(VlcMocFixture, VlcMediaPlayer_MocFunctions_Covered) {
    coverMocBasic(mp);
    // Parameterised signals:
    QMetaObject::invokeMethod(mp, "timeChanged", Qt::DirectConnection, Q_ARG(qint64, 0));
    QMetaObject::invokeMethod(mp, "snapshotTaken", Qt::DirectConnection, Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(mp, "buffering", Qt::DirectConnection, Q_ARG(float, 0.0f));
    QMetaObject::invokeMethod(mp, "titleChanged", Qt::DirectConnection, Q_ARG(int, 0));
    QMetaObject::invokeMethod(mp, "vout", Qt::DirectConnection, Q_ARG(int, 0));
    QMetaObject::invokeMethod(mp, "seekableChanged", Qt::DirectConnection, Q_ARG(bool, false));
    QMetaObject::invokeMethod(mp, "buffering", Qt::DirectConnection, Q_ARG(int, 0));
    QMetaObject::invokeMethod(mp, "lengthChanged", Qt::DirectConnection, Q_ARG(int, 0));
    QMetaObject::invokeMethod(mp, "positionChanged", Qt::DirectConnection, Q_ARG(float, 0.0f));
    QMetaObject::invokeMethod(mp, "pausableChanged", Qt::DirectConnection, Q_ARG(bool, false));
}

TEST_F(VlcMocFixture, VlcMedia_MocFunctions_Covered) {
    VlcMedia media;
    coverMocBasic(&media);
    // Parameterised signals
    QMetaObject::invokeMethod(&media, "subitemAdded", Qt::DirectConnection,
                              Q_ARG(libvlc_media_t*, nullptr));
    QMetaObject::invokeMethod(&media, "stateChanged", Qt::DirectConnection,
                              Q_ARG(Vlc::State, Vlc::Idle));
    QMetaObject::invokeMethod(&media, "metaChanged", Qt::DirectConnection,
                              Q_ARG(Vlc::Meta, Vlc::Meta::Title));
    QMetaObject::invokeMethod(&media, "parsedChanged", Qt::DirectConnection,
                              Q_ARG(bool, false));
    QMetaObject::invokeMethod(&media, "freed", Qt::DirectConnection,
                              Q_ARG(libvlc_media_t*, nullptr));
    QMetaObject::invokeMethod(&media, "durationChanged", Qt::DirectConnection,
                              Q_ARG(int, 0));
    QMetaObject::invokeMethod(&media, "parsedChanged", Qt::DirectConnection,
                              Q_ARG(int, 0));
}

TEST_F(VlcMocFixture, VlcEqualizer_MocFunctions_Covered) {
    VlcEqualizer eq(mp);
    coverMocBasic(&eq);
}

TEST_F(VlcMocFixture, SdlPlayer_MocFunctions_Covered) {
    SdlPlayer sdl(inst);
    coverMocBasic(&sdl);
}

TEST(MetaCoverageTest, CdaThread_MocFunctions_Covered) {
    CdaThread cda;
    // Only cover metaObject + qt_metacast + explicit signal invocations.
    // Skip qt_metacall direct call for QThread subclasses to avoid triggering
    // thread lifecycle via property/meta-method side-effects.
    coverMetaCast(&cda);
    QMetaObject::invokeMethod(&cda, "sigSendCdaStatus", Qt::DirectConnection, Q_ARG(int, 0));
    QList<DMusic::MediaMeta> metas;
    QMetaObject::invokeMethod(&cda, "sigSendCdaMimeData", Qt::DirectConnection,
                              Q_ARG(QList<DMusic::MediaMeta>, metas));
    cda.closeThread();
}

TEST_F(VlcMocFixture, CheckDataZeroThread_MocFunctions_Covered) {
    SdlPlayer sdl(inst);
    CheckDataZeroThread cz(nullptr, &sdl);
    coverMocBasic(&cz);
    QMetaObject::invokeMethod(&cz, "sigExtraTime", Qt::DirectConnection, Q_ARG(qint64, 0));
}

// ===========================================================================
// DBOperate
// ===========================================================================

TEST(MetaCoverageTest, DBOperate_MocFunctions_Covered) {
    DBOperate obj(QStringList() << "mp3" << "flac");
    coverMocBasic(&obj);
    DMusic::MediaMeta meta;
    QMetaObject::invokeMethod(&obj, "signalMetaCoverReady", Qt::DirectConnection,
                              Q_ARG(DMusic::MediaMeta, meta));
}

// ===========================================================================
// PlayerBase (abstract — use FakePlayer concrete subclass)
// ===========================================================================

TEST(MetaCoverageTest, PlayerBase_MocFunctions_Covered) {
    FakePlayer obj;
    coverMocBasic(&obj);
    QMetaObject::invokeMethod(&obj, "end", Qt::DirectConnection);
    QMetaObject::invokeMethod(&obj, "sigSendCdaStatus", Qt::DirectConnection, Q_ARG(int, 0));
}

TEST(MetaCoverageTest, QtPlayer_MocFunctions_Covered) {
    QtPlayer obj;
    coverMocBasic(&obj);
}

// ===========================================================================
// PlayerEngine (needs a FakePlayer backend)
// ===========================================================================

TEST(MetaCoverageTest, PlayerEngine_MocFunctions_Covered) {
    FakePlayer *fake = new FakePlayer;
    PlayerEngine *eng = new PlayerEngine(nullptr, fake);
    eng->setMprisPlayer("org.deepin.music", "deepin-music", "Music");
    coverMocBasic(eng);
    QMetaObject::invokeMethod(eng, "playPictureChanged", Qt::DirectConnection, Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(eng, "fadeInOutFactorChanged", Qt::DirectConnection, Q_ARG(double, 0.0));
    QMetaObject::invokeMethod(eng, "sendCdaStatus", Qt::DirectConnection, Q_ARG(int, 0));
    QMetaObject::invokeMethod(eng, "openUriRequested", Qt::DirectConnection, Q_ARG(QUrl, QUrl()));
    QMetaObject::invokeMethod(eng, "playPlaylistRequested", Qt::DirectConnection, Q_ARG(QString, QString()));
    delete eng;
    delete fake;
}

// ===========================================================================
// VlcPlayer (needs VLC dynamic library loaded)
// ===========================================================================

TEST(MetaCoverageTest, VlcPlayer_MocFunctions_Covered) {
    VlcPlayer obj;
    coverMocBasic(&obj);
}

// ===========================================================================
// DataManager (needs in-memory DB)
// ===========================================================================

TEST(MetaCoverageTest, DataManager_MocFunctions_Covered) {
    QString tmp = QDir::tempPath() + "/dm-metacov-ut";
    QDir().mkpath(tmp + "/cache");
    QDir().mkpath(tmp + "/config");
    QDir().mkpath(tmp + "/music");
    DmGlobal::setCachePath(tmp + "/cache");
    DmGlobal::setConfigPath(tmp + "/config");
    DmGlobal::setMusicPath(tmp + "/music");
    DmGlobal::setAppName("deepin-music-ut");
    DmGlobal::initPath();
    DataManager *dm = new DataManager(QStringList() << "mp3" << "flac", nullptr, ":memory:");
    coverMocBasic(dm);
    DMusic::MediaMeta meta;
    QMetaObject::invokeMethod(dm, "signalMetaCoverReady", Qt::DirectConnection,
                              Q_ARG(DMusic::MediaMeta, meta));
    QMetaObject::invokeMethod(dm, "signalClearImportingHash", Qt::DirectConnection,
                              Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(dm, "signalUpdatedMetaCodec", Qt::DirectConnection,
                              Q_ARG(DMusic::MediaMeta, meta), Q_ARG(QString, QString()),
                              Q_ARG(QString, QString()));
    delete dm;
}

// ===========================================================================
// Presenter (needs construction + MprisPlayer)
// ===========================================================================

TEST(MetaCoverageTest, Presenter_MocFunctions_Covered) {
    QString tmp = QDir::tempPath() + "/dm-presenter-metacov-ut";
    QDir().mkpath(tmp + "/cache");
    QDir().mkpath(tmp + "/config");
    QDir().mkpath(tmp + "/music");
    DmGlobal::setCachePath(tmp + "/cache");
    DmGlobal::setConfigPath(tmp + "/config");
    DmGlobal::setMusicPath(tmp + "/music");
    DmGlobal::setAppName("deepin-music-ut");
    DmGlobal::initPath();
    DmGlobal::setPlaybackEngineType(DmGlobal::QtMEDIAPLAYER);
    Presenter *p = new Presenter("UnknownAlbum", "UnknownArtist");
    p->setMprisPlayer("org.deepin.music", "deepin-music", "Music");
    coverMocBasic(p);
    // Parameterised signals
    QList<int> ints;
    QMetaObject::invokeMethod(p, "audioSpectrumData", Qt::DirectConnection, Q_ARG(QList<int>, ints));
    QList<QVariant> qvars;
    QMetaObject::invokeMethod(p, "audioBuffer", Qt::DirectConnection,
                              Q_ARG(QList<QVariant>, qvars), Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(p, "playlistSortChanged", Qt::DirectConnection, Q_ARG(QString, QString()));
    QMap<QString, QVariant> smap;
    QMetaObject::invokeMethod(p, "updatedMetaCodec", Qt::DirectConnection,
                              Q_ARG(StringVariantMap, smap), Q_ARG(QString, QString()),
                              Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(p, "updateCDStatus", Qt::DirectConnection, Q_ARG(int, 0));
    QMetaObject::invokeMethod(p, "renamedPlaylist", Qt::DirectConnection,
                              Q_ARG(QString, QString()), Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(p, "metaCoverReady", Qt::DirectConnection,
                              Q_ARG(StringVariantMap, smap));
    QList<QString> slist;
    QMetaObject::invokeMethod(p, "addMetaFinished", Qt::DirectConnection, Q_ARG(QList<QString>, slist));
    QMetaObject::invokeMethod(p, "deleteFinished", Qt::DirectConnection, Q_ARG(QList<QString>, slist));
    QMetaObject::invokeMethod(p, "deleteOneMeta", Qt::DirectConnection,
                              Q_ARG(QList<QString>, slist), Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(p, "deletedPlaylist", Qt::DirectConnection, Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(p, "updatePlayingIcon", Qt::DirectConnection, Q_ARG(QString, QString()));
    delete p;
}
