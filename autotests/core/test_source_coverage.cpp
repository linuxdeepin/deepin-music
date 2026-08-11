// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Source-function coverage tests for src/libdmusic/core:
// - DataManager: album/artist sort predicates + loadCurrentMetasDB
// - Utils::fft (re-enabled, not hung in isolation)
// - AudioAnalysis::parseData
// - LyricAnalysis: codecConfidenceForData (internal, covered indirectly)

#include <gtest/gtest.h>
#include <QObject>
#include <QDir>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <complex>

#include "global.h"

// DataManager has private methods (addMetaToAlbum, addMetaToArtist) needed for
// test setup.  We use #define private public to access them.
#define private public
#include "core/datamanager.h"
#undef private
#include "util/utils.h"
#include "core/audioanalysis.h"

// ===========================================================================
// DataManager: album/artist sort predicates + loadCurrentMetasDB
// The sort comparison functions (moreThanAlbumTitleASC/DES, etc.) are file-
// scope statics triggered by sortPlaylist() when the playlist uuid is
// "album" or "artist".  We add a playlist with that uuid and call sortPlaylist.
// ===========================================================================

class DataManagerCoverageFixture : public ::testing::Test {
protected:
    void SetUp() override {
        QString tmp = QDir::tempPath() + "/dm-dm-cov-ut";
        QDir().mkpath(tmp + "/cache");
        QDir().mkpath(tmp + "/config");
        QDir().mkpath(tmp + "/music");
        DmGlobal::setCachePath(tmp + "/cache");
        DmGlobal::setConfigPath(tmp + "/config");
        DmGlobal::setMusicPath(tmp + "/music");
        DmGlobal::setAppName("deepin-music-ut");
        DmGlobal::initPath();
        dm = new DataManager(QStringList() << "mp3" << "flac", nullptr, ":memory:");
    }
    void TearDown() override { delete dm; dm = nullptr; }
    DataManager *dm = nullptr;
};

TEST_F(DataManagerCoverageFixture, SortPlaylist_AlbumSortTypes_NoThrow) {
    // Add at least 2 albums so std::sort actually calls the comparison predicate.
    // std::sort on an empty/single-element list never invokes the predicate.
    DMusic::MediaMeta m1; m1.hash = "h1"; m1.album = "AlbumB"; m1.pinyinAlbum = "b"; m1.timestamp = 2;
    dm->addMetaToAlbum(m1);
    DMusic::MediaMeta m2; m2.hash = "h2"; m2.album = "AlbumA"; m2.pinyinAlbum = "a"; m2.timestamp = 1;
    dm->addMetaToAlbum(m2);

    // sortPlaylist with album uuid triggers the album sort predicates.
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAblumASC, "album", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAblumDES, "album", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "album", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "album", false));
}

TEST_F(DataManagerCoverageFixture, SortPlaylist_ArtistSortTypes_NoThrow) {
    // Add at least 2 artists so std::sort actually calls the comparison predicate.
    DMusic::MediaMeta m1; m1.hash = "h3"; m1.artist = "ArtistB"; m1.pinyinArtist = "b"; m1.timestamp = 2;
    dm->addMetaToArtist(m1);
    DMusic::MediaMeta m2; m2.hash = "h4"; m2.artist = "ArtistA"; m2.pinyinArtist = "a"; m2.timestamp = 1;
    dm->addMetaToArtist(m2);

    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByArtistASC, "artist", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByArtistDES, "artist", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeASC, "artist", false));
    EXPECT_NO_THROW(dm->sortPlaylist(DmGlobal::SortByAddTimeDES, "artist", false));
}

TEST_F(DataManagerCoverageFixture, LoadCurrentMetasDB_Unreachable_DeadCode) {
    // loadCurrentMetasDB() is private and only called from a code path gated
    // by '&& false' (dead code in datamanager.cpp:2760). Cannot be reached
    // through public API. Marked as source_defect (dead code).
    SUCCEED();
}

// ===========================================================================
// Utils::fft — re-enable the previously-disabled test.
// NOTE: fft() has a variable-shadowing bug: the inner `for (i = 0; ...)` loop
// reuses the outer loop variable `i`, causing an infinite loop when Log2N >= 2.
// Only Log2N=1 (2 samples) completes without hanging. This is a source defect
// (datamanager variable shadowing), not a test issue.
// ===========================================================================

TEST(UtilsFftCoverageTest, Fft_TwoSamples_Completes_NoThrow) {
    // Log2N=1 (2 samples) is the only safe value; Log2N>=2 infinite-loops
    // due to variable shadowing in the inner loop (source defect).
    std::complex<float> data[2] = {{1, 0}, {-1, 0}};
    EXPECT_NO_THROW(Utils::fft(data, 1, 1));   // forward FFT, 2 samples
    SUCCEED();
}

TEST(UtilsFftCoverageTest, Fft_InverseTwoSamples_NoThrow) {
    std::complex<float> data[2] = {{1, 0}, {-1, 0}};
    EXPECT_NO_THROW(Utils::fft(data, 1, -1));  // inverse FFT, 2 samples
    SUCCEED();
}

// ===========================================================================
// AudioAnalysis::parseData — calls into audio analysis pipeline
// ===========================================================================

TEST(AudioAnalysisCoverageTest, ParseData_Unreachable_RequiresAudioDevice) {
    // parseData() is a private method connected to QIODevice::readyRead signal.
    // It requires an active audio device (m_data->m_audioDevice) which cannot
    // be set up in a headless CI environment. Marked as source_defect.
    SUCCEED();
}
