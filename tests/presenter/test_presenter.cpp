#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>
#include <DHiDPIHelper>
#include "presenter.h"
#include "presenter_p.h"
#include "player.h"
#include "searchmeta.h"

DWIDGET_USE_NAMESPACE

TEST(Transfer, tf)
{
    Transfer  tr;
    PlaylistMeta musiclistinfo;
    musiclistinfo.sortMetas = QStringList() << "a.mp3" << "b.mp3" << "c.mp3";
    for (QString str : musiclistinfo.sortMetas) {
        musiclistinfo.metas[str] = MetaPtr(new MediaMeta);
    }
    PlaylistPtr pptr(new Playlist(musiclistinfo));
    MetaPtrList  mplst;
    MetaPtr  meta1 = MetaPtr(new MediaMeta);
    meta1->title = "a.mp3";
    MetaPtr  meta2 = MetaPtr(new MediaMeta);
    meta2->title = "b.mp3";
    MetaPtr  meta3 = MetaPtr(new MediaMeta);
    meta3->title = "c.mp3";
    mplst.append(meta1);
    mplst.append(meta2);
    mplst.append(meta3);

    tr.onMusicListAdded(pptr, mplst);
}

TEST(Presenter, pst)
{
    Player::instance();
    Presenter *p =  new Presenter;
    MetaPtr  meta1 = MetaPtr(new MediaMeta);
    meta1->searchID = "bensound-sunny.mp3";
    //p->openUri(QUrl("file://usr/share/music/bensound-sunny.mp3"));
    QStringList strlst = QStringList() << "bbb" << "aa" << "aaa";
    p->removeListSame(&strlst);
    DMusic::SearchMeta meta;
    meta.id = "bensound-sunny.mp3";
    p->onChangeSearchMetaCache(meta1, meta);
    p->initMpris(new MprisPlayer());
}

