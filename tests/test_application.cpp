#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QDebug>
#include <QAction>
#include <QProcess>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QPixmap>
#include <QTestEventList>
#include <QPointF>
#include <QWheelEvent>
#include <QTest>

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>
#include <DHiDPIHelper>

#include "musicapp.h"
#include "widget/soundvolume.h"
#include "widget/searchedit.h"
#include "widget/cover.h"
#include "widget/infodialog.h"
#include "widget/ddropdown.h"
#include "widget/delegate/lyriclinedelegate.h"
#include "widget/delegate/musicinfoitemdelegate.h"
#include "widget/delegate/musiclistdatadelegate.h"
#include "widget/delegate/musicinfoitemdelegate_p.h"
#include "widget/delegate/musicitemdelegate_p.h"
#include "widget/delegate/musicsearchlistdelegate.h"
#include "widget/delegate/playitemdelegate.h"
#include "widget/delegate/playitemdelegate_p.h"
#include "widget/closeconfirmdialog.h"
#include "widget/dequalizerdialog.h"
#include "widget/filter.h"
#include "widget/label.h"
#include "widget/listview.h"
#include "widget/lyriclabel.h"
#include "widget/lyricview.h"
#include "widget/modebuttom.h"
#include "widget/searchresult.h"
#include "widget/model/musiclistdatamodel.h"
#include "widget/model/musiclistinfomodel.h"
#include "widget/model/musiclistmodel.h"
#include "widget/model/musicsearchlistmodel.h"
#include "widget/model/playlistmodel.h"
#include "widget/musicboxbutton.h"
#include "widget/musiciconbutton.h"
#include "widget/musicimagebutton.h"
#include "widget/musiclistdataview.h"
#include "widget/musiclistdialog.h"
#include "widget/musiclistinfoview.h"
#include "widget/musicsearchlistview.h"
#include "widget/playlistview.h"
#include "widget/pushbutton.h"
#include "widget/searchedit.h"
#include "widget/searchlyricswidget.h"
#include "widget/titlebarwidget.h"
#include "widget/tooltips.h"
#include "widget/waveformscale.h"
//#include "../music-player/view/footerwidget.h"
#include "importwidget.h"
#include "loadwidget.h"
#include "lyricwidget.h"
#include "mainframe.h"
#include "shortcut.h"
#include "playlistwidget.h"
#include "musiclyricwidget.h"
#include "medialibrary.h"
#include "mediadatabase.h"
#include "mediameta.h"
#include "metadetector.h"
#include "net/geese.h"
#include "util/basetool.h"
#include "util/cueparser.h"
#include "util/encodingdetector.h"
#include "util/pinyinsearch.h"
#include "core/mediadatabasewriter.h"
#include "metabufferdetector.h"
#include "metasearchservice.h"
#include "player.h"
#include "playlist.h"
#include "pluginmanager.h"
#include "fft.h"
#include "inotifyengine.h"
#include "inotifyfiles.h"
#include "lyric.h"
#include "musiclyric.h"

#include <vlc/vlc.h>
#include "vlc/Audio.h"
#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"

#include "widget/closeconfirmdialog.h"
#include "helper/widgethellper.h"
//#include "widget/playlistitem.h"
#include "mpris.h"
#include "mprismanager.h"
#include "mpriscontroller.h"
#include "mpriscontroller_p.h"
#include "mprisplayer_p.h"
#include "mprisplayer.h"
#include "speech/speechCenter.h"
#include "speech/exportedinterface.h"
#include "presenter.h"
#include "presenter_p.h"
#include "threadpool.h"
#include "volumemonitoring.h"
#include "vlc/vlcdynamicinstance.h"
#include "dbusutils.h"
#include "footerwidget.h"

MetaPtr globMetaPtr(new MediaMeta());

int sum(int a, int b)
{
    return a + b;
}

TEST(qqq, www)
{
    ASSERT_EQ(2, sum(1, 1));
}

TEST(Application, volume)
{
    SoundVolume *a = new  SoundVolume() ;
//    ASSERT_EQ(20, a->radius());
    a->setRadius(5);
//    ASSERT_EQ(5, a->radius());
    a->deleyHide();
    a->onVolumeChanged(10);
    ASSERT_EQ(10, a->volume());
}

TEST(SearchEdit, isNull)
{
    SearchEdit *a = new  SearchEdit() ;
    SearchResult *b = new SearchResult;
    a->setResultWidget(b);
    a->onFocusIn();
    a->onFocusOut();
    a->onTextChanged();
    a->onReturnPressed();
    bool isNull = a->curPlaylistPtr().isNull();
    ASSERT_TRUE(isNull);
}

TEST(Cover, setcolor)
{
    Cover *a = new  Cover() ;
    QPixmap pix;
    QColor color(255, 255, 255);
    a->setBorderColor(color);
    a->setShadowColor(color);
    a->radius();
    a->borderColor();
    a->shadowColor();
    a->setCoverPixmap(pix);
    a->setRadius(1);
    ASSERT_EQ(1, a->radius());
    ASSERT_EQ(color, a->borderColor());
    ASSERT_EQ(color, a->shadowColor());
//    delete  a;
//    a = nullptr;
}

TEST(InfoDialog, isRunning4)
{
    InfoDialog *a = new  InfoDialog() ;
    MetaPtr meta ;
//    a->updateInfo(meta);
    a->setThemeType(0);
//    a->expand(true);
}

TEST(DDropdown, setStr)
{
    DDropdown *a = new  DDropdown() ;
    QString str = "music";
    a->setStatus(str);
    a->status();
    a->actions();
    a->setText(str);
    a->setCurrentAction(0);
    ASSERT_EQ(str, a->status());
}

TEST(LyricLineDelegate, isRunning6)
{
    LyricLineDelegate *a = new  LyricLineDelegate() ;
//    delete a;
//    a = nullptr;
}

TEST(MusicInfoItemDelegate, isRunning7)
{
    MusicInfoItemDelegate *a = new  MusicInfoItemDelegate() ;
    Q_UNUSED(a)
}

TEST(MusicListDataDelegate, isRunning8)
{
    MusicListDataDelegate *a = new  MusicListDataDelegate() ;
    QWidget *parent = new QWidget;
    QStyleOptionViewItem option;
    QModelIndex index;
    a->initStyleOption(&option, index);
    a->createEditor(parent, option, index);
    a->setEditorData(parent, index);
}

TEST(MusicInfoItemDelegatePrivate, setcolor)
{
    MusicInfoItemDelegatePrivate *a = new  MusicInfoItemDelegatePrivate() ;
    QStyleOptionViewItem option;
    a->textColor();
    a->titleColor();
    a->highlightText();
    a->background();
    a->alternateBackground();
    a->highlightedBackground();
    a->playingIcon();
    a->highlightPlayingIcon();
    QColor color(255, 255, 255);
    a->setTextColor(color);
    a->setTitleColor(color);
    a->setHighlightText(color);
    a->setBackground(color);
    a->setAlternateBackground(color);
    a->setHighlightedBackground(color);
    a->setPlayingIcon("aaa");
    a->setHighlightPlayingIcon("a");
    ASSERT_EQ(color, a->textColor()) << " set color success";
}

//TEST(MusicItemDelegatePrivate, isRunning10)
//{
//    MusicItemDelegatePrivate *a = new  MusicItemDelegatePrivate() ;
//    Q_UNUSED(a)
//}

TEST(MusicSearchListDelegate, isRunning11)
{
    MusicSearchListDelegate *a = new  MusicSearchListDelegate() ;
    Q_EMIT a->SearchClear();
}

TEST(PlayItemDelegate, isRunning12)
{
    PlayItemDelegate *a = new  PlayItemDelegate() ;
    Q_UNUSED(a)
}

TEST(PlayItemDelegatePrivate, setcolor)
{
    PlayItemDelegatePrivate *a = new  PlayItemDelegatePrivate() ;
    a->textColor();
    a->titleColor();
    a->highlightText();
    a->background();
    a->alternateBackground();
    a->highlightedBackground();
    a->playingIcon();
    a->highlightPlayingIcon();
    QColor color(255, 255, 255);
    a->setTextColor(color);
    a->setTitleColor(color);
    a->setHighlightText(color);
    a->setBackground(color);
    a->setAlternateBackground(color);
    a->setHighlightedBackground(color);
    a->setPlayingIcon("s");
    a->setHighlightPlayingIcon("s");
    ASSERT_EQ(color, a->textColor()) << " set color success";
}

TEST(CloseConfirmDialog, isRunning14)
{
    CloseConfirmDialog *a = new  CloseConfirmDialog() ;
    Q_UNUSED(a)
}

TEST(DequalizerDialog, isRunning15)
{
    DequalizerDialog *a = new  DequalizerDialog() ;
    Q_UNUSED(a)
}

TEST(HoverFilter, isRunning16)
{
    HoverFilter *a = new  HoverFilter() ;
    HoverShadowFilter *b = new HoverShadowFilter();
    HintFilter *c = new HintFilter();
    c->hideAll();
    Q_UNUSED(a)
    ASSERT_FALSE(b == nullptr);
}

TEST(Label, isRunning17)
{
    Label *a = new  Label() ;
    Label *b = new Label("asd", nullptr);
    Q_UNUSED(a)
    ASSERT_FALSE(b == nullptr);
}

TEST(ListView, isRunning18)
{
    ListView *a = new  ListView() ;
    QTestEventList event;
    event.addMouseClick(Qt::MouseButton::LeftButton);
    event.simulate(a);
    event.clear();
    Q_UNUSED(a)
}

TEST(LyricLabel, isRunning19)
{
    LyricLabel *a = new  LyricLabel(true) ;
    Q_UNUSED(a)
}

TEST(LyricView, viewMode)
{
    LyricView *a = new  LyricView() ;
    QWheelEvent event(a->pos(), 1, Qt::MouseButtons(Qt::MouseButtonMask), Qt::KeyboardModifier::NoModifier);
    QApplication::sendEvent(a, &event);
    a->optical();
    ASSERT_FALSE(a->viewMode());
}

TEST(ModeButton, mode)
{
    ModeButton *a = new  ModeButton() ;
    QStringList modes;
    modes << ":/mpimage/light/normal/sequential_loop_normal.svg"
          << ":/mpimage/light/normal/single_tune_circulation_normal.svg"
          << ":/mpimage/light/normal/cross_cycling_normal.svg";
    QStringList pressModes;
    pressModes << ":/mpimage/light/press/sequential_loop_press.svg"
               << ":/mpimage/light/press/single_tune_circulation_press.svg"
               << ":/mpimage/light/press/cross_cycling_press.svg";

    a->setModeIcons(modes, pressModes);
    a->mode();
    a->setTransparent(true);
    a->setMode(1);
    a->update();
    ASSERT_EQ(1, a->mode());
}

TEST(MusicListDataModel, isRunning22)
{
    MusicListDataModel *a = new  MusicListDataModel() ;
    PlaylistPtr playlist;
    a->setPlaylist(playlist);
    a->playlist();
    ASSERT_TRUE(a->playlist() == nullptr);
}

TEST(MusiclistInfomodel, isRunning23)
{
    MusiclistInfomodel *a = new  MusiclistInfomodel() ;
    a->playlist();
}

TEST(MusiclistModel, isRunning24)
{
    MusiclistModel *a = new  MusiclistModel() ;
    Q_UNUSED(a)
}

TEST(MusicSearchListModel, isRunning25)
{
    MusicSearchListModel *a = new  MusicSearchListModel() ;
    a->playlist();
}

extern void margeDatabase();
TEST(PlaylistModel, findmusic)
{
//    MediaDatabase::instance()->init();
    auto metalist = MediaDatabase::instance()->allmetas();
    margeDatabase();
    MediaDatabase::instance()->searchMediaTitle("a", 1);
    MediaDatabase::instance()->searchMediaMeta("a", 1);
    MediaDatabase::instance()->searchMediaPath("/usr/share/music/", 1);
    PlaylistMeta all;
    MediaDatabase::instance()->addPlaylist(all);
    MediaDatabase::instance()->updatePlaylist(all);
    MediaDatabase::instance()->removePlaylist(all);
    MediaDatabase::instance()->deleteMusic(globMetaPtr, all);
    MediaDatabase::instance()->playlistExist("all");
    MediaDatabase::instance()->mediaMetaExist("all");
    MediaMeta *meta = new MediaMeta();
    if (metalist.isEmpty()) {
        meta->hash = "music";
    } else
        *meta = metalist.at(0);
//    QSharedPointer<MediaMeta> ptr(meta);
    PlaylistModel *a = new  PlaylistModel() ;
    PlaylistModel *b = new  PlaylistModel(1, 1, nullptr) ;
    Q_UNUSED(b);
    QModelIndex index;
    a->meta(index);
    a->playlist();

    //datastream
    QDataStream steam;
    steam << (QDataStream(), MetaPtr(meta));
    //steam>> (QDataStream(),ptr);
//    a->findIndex(ptr);
}

TEST(MusicBoxButton, boxbutton)
{
    MusicBoxButton *a = new  MusicBoxButton("") ;
    MusicBoxButton *b = new  MusicBoxButton("", "", "", "", "") ;
    a->setTransparent(true);
    a->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");
    b->setAutoChecked(true);
    b->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");
}

TEST(MusicIconButton, IconButton)
{
    MusicIconButton *a = new  MusicIconButton() ;
    MusicIconButton *b = new  MusicIconButton("", "", "", "") ;
    a->setTransparent(true);
    a->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");
    b->setAutoChecked(true);
    b->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");

}

TEST(MusicImageButton, isRunning30)
{
    MusicImageButton *a = new  MusicImageButton() ;
    MusicImageButton *b = new  MusicImageButton("", "", "", "") ;
    QVariant value;
    value.setValue(1);
    b->setPropertyPic("a", value, "a", "c", "v", "");
    a->setPropertyPic("", "", "", "");
    a->setTransparent(true);
    a->setAutoChecked(true);
}

TEST(MusicListDataView, isRunning31)
{
    MusicListDataView *a = new  MusicListDataView() ;
    a->playlist();
    a->setViewModeFlag(QListView::ViewMode::ListMode);
    a->playing();
    a->hoverin();
//    a->playingState();
    a->playMusicTypePtrList();
    a->setThemeType(1);
    a->getThemeType();
    a->getPlayPixmap();
    a->getSidebarPixmap();
    a->getAlbumPixmap();
    a->updateList();
    ASSERT_EQ(0, a->rowCount());
//    ASSERT_EQ(0, a->listSize());
}

TEST(MusicListDialog, isRunning32)
{
    MusicListDialog *a = new  MusicListDialog() ;
    a->setThemeType(1);
}

TEST(MusicListInfoView, isRunning33)
{
    MusicListInfoView *a = new  MusicListInfoView() ;
    MetaPtr meta;
    a->activingMeta();
    a->firstMeta();
    a->playlist();
//    a->findIndex(meta);
    a->curName();
    a->setPlaying(meta);
    a->playing();
    a->setThemeType(1);
    a->getThemeType();
    a->getPlayPixmap();
    a->getSidebarPixmap();
    a->allMetaNames();
}

TEST(MusicSearchListview, isRunning34)
{
    MusicSearchListview *a = new  MusicSearchListview() ;
    a->playlist();
    a->rowCount();
    a->listSize();
    a->getIndexInt();
    a->playMetaList();
    a->playMusicTypePtrList();
    a->setThemeType(1);
    a->getThemeType();
    a->getSearchText();
    a->setCurrentIndexInt(1);
    a->getPlayPixmap();
    a->getSidebarPixmap();
    a->getAlbumPixmap();
    a->updateList();
    a->keyChoice();
}
TEST(ModelMake, isRunning35)
{
    ModelMake *a = new  ModelMake() ;
    PlayListView *b = new PlayListView(true, true);
    b->activingMeta();
    b->playlist();
    b->playMetaPtrList();
    b->setThemeType(1);
    b->getThemeType();
    b->getPlayPixmap();
    b->getSidebarPixmap();
    b->getAlbumPixmap();
    b->rowCount();
    b->firstHash();
    Q_UNUSED(a)
}
TEST(PushButton, isRunning36)
{
    PushButton *a = new  PushButton() ;
    Q_UNUSED(a)
}
TEST(SearchEdit, isRunning37)
{
    SearchEdit *a = new  SearchEdit() ;
    SearchResult *b = new SearchResult();
    a->curPlaylistPtr();
    a->setResultWidget(b);
    a->onFocusIn();
    a->onFocusOut();
    a->onTextChanged();
    a->onReturnPressed();
    Q_UNUSED(a)
}
TEST(SearchLyricsWidget, isRunning38)
{
    lyric stru;
    SearchLyricsWidget *a = new  SearchLyricsWidget("") ;
    a->setSearchDir("/home/");
    a->setDefault("a.mp3", "a");
    a->setThemeType(1);
    a->searchLyrics();
}
TEST(TitlebarWidget, isRunning39)
{
    TitlebarWidget *a = new  TitlebarWidget() ;
    SearchResult *b = new SearchResult();
    a->exitSearch();
    a->clearSearch();
    a->setSearchEnable();
    a->setResultWidget(b);
    a->setViewname("a");
    a->setEditStatus();
    a->onSearchAborted();
}

TEST(ToolTips, isRunning40)
{
    ToolTips *a = new  ToolTips("") ;
    a->radius();
    a->borderColor();
    a->background();
    a->setText("");
    a->setRadius(1);
}

TEST(WaveformScale, isRunning41)
{
    WaveformScale *a = new  WaveformScale();
    a->show();
    a->setValue(1);
    a->hide();
}

TEST(ImportWidget, isRunning43)
{
    ImportWidget *a = new  ImportWidget() ;
    a->showWaitHint();
    a->showImportHint();
    a->slotTheme(1);
}

TEST(LoadWidget, isRunning44)
{
    LoadWidget *a = new  LoadWidget() ;
    Q_UNUSED(a)
}

TEST(LyricWidget, isRunning45)
{
    LyricWidget *a = new  LyricWidget() ;
    a->updateUI();
    a->defaultCover();
    a->backgroundColor();
    a->setDefaultCover("a");
    a->onProgressChanged(1, 1);
    PlaylistMeta playlist;
    PlaylistPtr playlist1(new Playlist(playlist));
    a->onMusicPlayed(playlist1, globMetaPtr);
    a->onMusicStop(playlist1, globMetaPtr);
    DMusic::SearchMeta sea;
    DMusic::SearchMeta sea1("1");
    QString lyr = "home/zhangyong/Music/123.lrc";
    a->onLyricChanged(globMetaPtr, sea, lyr.toUtf8());
    a->onCoverChanged(globMetaPtr, sea, lyr.toUtf8());
    QColor color(255, 255, 255, 0);
    a->setBackgroundColor(color);
    a->onUpdateMetaCodec(globMetaPtr);
}

TEST(Shortcut, isRunning47)
{
    MusicSettings::init();
    MusicSettings::value("");
    Shortcut *a = new  Shortcut() ;
    ShortcutItem b("", "");
    ShortcutGroup c;
    a->toStr();
}

TEST(PlayListWidget, isRunning48)
{
    PlayListWidget *a = new  PlayListWidget() ;
    a->curPlaylist();
    a->slotTheme(1);
}

TEST(MUsicLyricWidget, isRunning49)
{
    MUsicLyricWidget *a = new  MUsicLyricWidget() ;
    a->updateUI();
    a->defaultCover();
    a->slotTheme(1);
    a->onProgressChanged(1, 1);
    a->setDefaultCover("a");
    a->onsearchBt();
    a->slotonsearchresult("a");
}

TEST(MediaMeta, metabase)
{
    MediaMeta *meta = new MediaMeta;
    meta->title = "music";
    meta->artist = "music";
    meta->album = "music";
    meta->updateSearchIndex();
    meta->updateCodec("utf8");
    QFileInfo fileinfo("/usr/share/music");
    meta->fromLocalFile(fileinfo);
    DMusic::sizeString(1025);
    DMusic::sizeString(1024 * 1024 * 2);
    DMusic::sizeString(1024 * 1024 * 1024 * 2);
    ASSERT_FALSE(meta->getCoverData("/usr/bin") != "");
}

TEST(MediaMeta, length)
{
    MediaMeta *meta = new MediaMeta;
    meta->updateSearchIndex();
    meta->updateCodec("utf8");
    QFileInfo fileinfo("/usr/share/music");
    meta->fromLocalFile(fileinfo);
    QString path = DMusic::filepathHash("/usr/share/music/bensound-sunny.mp3");
    QString length = DMusic::lengthString(10000);
    ASSERT_TRUE(length.length() > 0);
}

TEST(MetaDetector, metadata)
{
    MetaDetector *metadet = new MetaDetector;
    metadet->init();
    auto meta = MetaPtr(new MediaMeta);
    meta->hash = "as123gxws5";
    MediaMeta *meta_ = new MediaMeta;
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QFileInfo fileinfo(path);
    metadet->updateMetaFromLocalfile(meta.data(), fileinfo);
    metadet->getCoverData(path, path, meta->hash);
    metadet->updateCueFileTagCodec(meta_, fileinfo, "utf8");
    metadet->updateMediaFileTagCodec(meta_, "utf8", false);
    QVector<float> vec = metadet->getMetaData(path);
    ASSERT_FALSE(vec.isEmpty());
}

TEST(BaseTool, basetool)
{
//    QString path = "/usr/share/music/bensound-sunny.mp3";
    QString lrc = "/home/zhangyong/Music/123.lrc";
    QFile fin(lrc);
    if (fin.open(QIODevice::ReadOnly)) {
        QByteArray data = fin.readAll();
        fin.close();
        QByteArray encoding = BaseTool::detectEncode(data, lrc);//歌词
        ASSERT_TRUE(encoding.size() > 0);
    }
}

extern qint64 timeframe2mtime(long frame);
//extern void Libcue::time_frame_to_msf(long frame, int *m, int *s, int *f);
TEST(CueParser, cue)
{
    QString path = "/usr/share/music/bensound-sunny.mp3";
    DMusic::CueParser *cue = new DMusic::CueParser(path, "utf8");
    cue->metalist();
    QString str = cue->mediaFilepath();
    QString str1 = cue->cueFilepath();
    int m = 1/*, s = 1, f = 1*/;
//    Libcue::time_frame_to_msf(1, &m, &s, &f);
    timeframe2mtime(m);
    ASSERT_EQ(path, str1);
}

TEST(EncodingDetector, encode)
{
    DMusic::EncodingDetector::detectEncodings("");
}

TEST(PinyinSearch, pinyin)
{
    QChar char_ = 'z';
    QString str = "deepin-music";
    bool v = DMusic::PinyinSearch::isChinese(char_);
    DMusic::PinyinSearch::simpleChineseSplit(str);
//    DMusic::PinyinSearch::toChinese(str);
    ASSERT_FALSE(v);
}

TEST(MediaDatabaseWriter, mediawriter)
{
    MetaPtr meta = globMetaPtr;
    MetaPtrList metalist;
    metalist << meta;
    PlaylistMeta listmeta;
    listmeta.uuid = "z";
    listmeta.displayName = "music";
    MediaDatabaseWriter *writer = new MediaDatabaseWriter;
    writer->insertMusic(meta, listmeta);
    writer->updateMediaMeta(meta);
    writer->removeMediaMeta(meta);
    writer->addMediaMetaList(metalist);
    writer->updateMediaMetaList(metalist);
    writer->removeMediaMetaList(metalist);
//    writer->initDataBase();
}

TEST(MediaLibrary, medialib)
{
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QStringList list;
    list << path;
    MediaLibrary::instance()->init();
    MediaLibrary::instance()->meta("z");
    MediaLibrary::instance()->contains("z");
    bool v = MediaLibrary::instance()->isEmpty();
    MediaLibrary::instance()->importFile(path);
    MediaLibrary::instance()->importMedias("all", list);
    ASSERT_FALSE(v);
}

TEST(MetaBufferDetector, metabuf)
{
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QString hash = "z";
    MetaBufferDetector *buf = new MetaBufferDetector;
    buf->onBufferDetector(path, hash);
    buf->onClearBufferDetector();
    buf->run();
    QTest::qWait(300);
}

extern QString cacheCoverPath(const MetaPtr meta);
extern QString cacheCoverPath(const QString &searchID);
TEST(MetaSearchService, metaservice)
{
    MetaSearchService   *lyricService = MetaSearchService::instance();
//    cacheLyricPath(globMetaPtr);
    cacheCoverPath(globMetaPtr);
//    cacheLyricPth("");
    cacheCoverPath("");
    lyricService->init();
    lyricService->searchContext("");
    lyricService->coverData(globMetaPtr);
    lyricService->lyricData(globMetaPtr);
    lyricService->coverUrl(globMetaPtr);
    lyricService->lyricUrl(globMetaPtr);
    lyricService->searchMeta(globMetaPtr);
}

TEST(Player, play)
{
    Player::instance()->init();
    Player::instance()->playNextMeta();
    Player::instance()->pause();
    Player::instance()->pauseNow();
    Player::instance()->stop();
    Player::instance()->status();
    Player::instance()->activeMeta();
    Player::instance()->curPlaylist();
    Player::instance()->activePlaylist();
    Player::instance()->supportedFilterStringList();
    Player::instance()->supportedSuffixList();
    Player::instance()->supportedMimeTypes();
    Player::instance()->canControl();
    Player::instance()->position();
    Player::instance()->volume();
    Player::instance()->mode();
    Player::instance()->muted();
    Player::instance()->duration();
    Player::instance()->fadeInOutFactor();
    Player::instance()->fadeInOut();
    Player::instance()->playOnLoaded();
    Player::instance()->setCanControl(false);
    Player::instance()->setPosition(64);
    Player::instance()->setMode(Player::PlaybackMode::RepeatAll);
    Player::instance()->setVolume(50);
    Player::instance()->setMuted(true);
    Player::instance()->setLocalMuted(true);
    Player::instance()->setDbusMuted(false);
    Player::instance()->setFadeInOutFactor(2);
    Player::instance()->setFadeInOut(false);
    Player::instance()->setPlayOnLoaded(false);
    Player::instance()->musicFileMiss();
    Player::instance()->setEqualizerEnable(true);
    Player::instance()->setEqualizerpre(1);
    Player::instance()->setEqualizerbauds(1, 1);
    Player::instance()->setEqualizerCurMode(1);
    Player::instance()->isValidDbusMute();
    ASSERT_FALSE(Player::instance() == nullptr);
}

extern const MetaPtrList shuffle(MetaPtrList &&musiclist, int seed);
TEST(Playlist, playlist)
{
    PlaylistMeta meta;
    meta.uuid = "z";
    meta.displayName = "music";
    Playlist *list = new Playlist(meta);
    list->load();
    list->prev(globMetaPtr);
    list->id();
    list->displayName();
    list->icon();
    list->readonly();
    list->editmode();
    list->hide();
    list->isEmpty();
    list->canNext();
    list->length();
    list->sortType();
    list->sortID();
    list->active();
    list->setActive(true);
    list->first();
    list->music(1);
    list->music("");
    list->playing();
    list->playingStatus();
    list->setPlayingStatus(false);
    list->index("z");
    list->allmusic();
    list->playMusicTypePtrList();
    list->changePlayMusicTypeOrderType();
    list->setSearchStr("");
    list->searchStr();
    list->setViewMode(0);
    list->viewMode();
    list->clearTypePtr();
    shuffle(list->allmusic(), 1);
    list->shuffleNext(globMetaPtr);
    list->shufflePrev(globMetaPtr);
    list->isLast(globMetaPtr);
    list->reset(list->allmusic());
    list->appendMusicList(list->allmusic());
    list->updateMeta(globMetaPtr);
    list->removeMusicList(list->allmusic());
    list->removeOneMusic(globMetaPtr);
    list->resort();
    QMap<QString, int > index;
    index.insert("", 0);
    list->saveSort(index);
    QStringList listsortMeta;
    listsortMeta << "music";
    list->playMusicTypeToMeta("index", listsortMeta);
    ASSERT_EQ(0, list->viewMode());
}

TEST(PluginManager, plugin)
{
    PluginManager *plu = new PluginManager;
    plu->init();
}

//TEST(CFFT, fft)
//{
//    complex<float> *sample;
//    sample = new complex<float>[100];
//    int log2N = static_cast<int>(log2(100 - 1) + 1);
//    CFFT::process(sample, log2N, -1);
//    ASSERT_TRUE(sample != nullptr);
//}

TEST(InotifyEngine, inottify)
{
    InotifyEngine *inotify = new InotifyEngine;
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QStringList list;
    list << path;
    inotify->addPath(path);
    inotify->addPaths(list);
    Q_EMIT inotify->fileRemoved(path);
}

TEST(InotifyFiles, inofile)
{
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QStringList list;
    list << path;
    InotifyFiles *inofile = new InotifyFiles;
    inofile->start();
    inofile->clear();
    inofile->addPath(path);
    inofile->addPaths(list);
    inofile->scanFiles();
}

TEST(Lyric, lyric)
{
    QString path = "/usr/zhangyong/Music/123.lrc";
    Lyric ly = parseLrc(path);
    ASSERT_FALSE(ly.hasTime);
}

TEST(MusicLyric, mly)
{
    MusicLyric *mly = new MusicLyric;
    QString path = "/home/zhangyong/Music/123.lrc";
    mly->getFromFile(path);
//    QString linestr = mly->getLineAt(0);
    mly->getFromFileOld(path);
    mly->getCount();
    mly->getIndex(2);
    mly->getPostion(2);
    mly->getHeadFromFile(path);
}

//vlc
DCORE_USE_NAMESPACE
TEST(VlcAudio, vlcaud)
{
    VlcInstance *instance = new VlcInstance(VlcCommon::args(), nullptr);
    VlcMediaPlayer *player = new VlcMediaPlayer(instance);
    player->equalizer()->setPreamplification(12);
    VlcMedia *vmedia = new VlcMedia("/usr/share/music/bensound-sunny.mp3", true, instance);
    vmedia->initMedia("/usr/share/music/bensound-sunny.mp3", true, instance);
    vmedia->parse();
    vmedia->parsed();
    vmedia->currentLocation();
    QString path = "/usr/share/music/";
    QString name = "bensound-sunny.mp3";
    Vlc::Mux m = Vlc::TS;
    Vlc::AudioCodec ac = Vlc::MP3;
    Vlc::VideoCodec vc = Vlc::NoVideo;
    vmedia->record(name, path, m, ac, vc, false);
    vmedia->record(name, path, m, false);
    vmedia->record(name, path, m, ac, vc, 1, 1, 1, false);
    vmedia->setProgram(2);
    vmedia->setOption("mp3");
    vmedia->setOptions(QStringList() << "1" << "2");

    VlcAudio *audio = new VlcAudio(player);
    audio->setVolume(50);
    audio->setTrack(1);
    audio->toggleMute();
    audio->setChannel(Vlc::AudioChannel::RStereo);
    audio->setMute(true);
    audio->track();
    QStringList list = audio->trackDescription();
    QList<int> li = audio->trackIds();
    QMap<int, QString> map1 = audio->tracks();
    audio->channel();

    player->play();

    VolumeMonitoring vot(nullptr);
    vot.needSyncLocalFlag();
    vot.syncLocalFlag(true);
    vot.start();
    vot.readSinkInputValid();
    vot.timeoutSlot();
    player->pause();
    player->setTime(10000);
    player->resume();
    player->seekable();
    player->togglePause();
    player->core();
    player->state();
    player->setPosition(5000);
    player->position();
    float rate = 1;
    player->setPlaybackRate(rate);
    rate = player->playbackRate();
    vmedia->core();
    vmedia->currentLocation();

    vmedia->state();
    vmedia->duplicate("ssssstx", "/usr/share/music/bensound-sunny.mp3", Vlc::TS);
    vmedia->merge("ssssstx", "/usr/share/music/bensound-sunny.mp3", Vlc::TS);
    ASSERT_TRUE(audio->getMute());

    //player->deleteLater();
    //instance->deleteLater();
}

TEST(VLC, vlc)
{
    Vlc *vlc = new Vlc;
    vlc->audioCodec();
    vlc->audioOutput();
    vlc->audioOutputHuman();
    vlc->deinterlacing();
    vlc->mux();
    vlc->ratio();
    vlc->ratioHuman();
    vlc->scale();
    vlc->videoCodec();
    vlc->videoOutput();
    vlc->videoOutputHuman();
    QStringList list = vlc->logLevel();
    ASSERT_EQ("debug", list.at(0));
}

TEST(closeconfirm, confirm)
{
    CloseConfirmDialog *close = new CloseConfirmDialog;
    bool a = close->isRemember();
    close->closeAction();
    ASSERT_FALSE(a);
}

TEST(widgethelper, helper)
{
    QImage image("");
    QWidget *widget1 = new QWidget;
    QWidget *widget2 = new QWidget;
    WidgetHelper::blurImage(image, 1);
    WidgetHelper::cropRect(image, QSize(1, 1));
    WidgetHelper::coverPixmap(" ", QSize(1, 1));
    WidgetHelper::slideRight2LeftWidget(widget1, widget2, 1);
    WidgetHelper::slideBottom2TopWidget(widget1, widget2, 1);
    WidgetHelper::slideTop2BottomWidget(widget1, widget2, 1);
    WidgetHelper::workaround_updateStyle(widget1, "");
}

TEST(mpris, mp)
{
    Mpris mp;
    Mpris::metadataToString(Mpris::Title);
}

TEST(MprisManager, control)
{
    MprisManager *manager = new MprisManager;
    manager->raise();
    manager->next();
    manager->pause();
    manager->play();
    manager->playPause();
    manager->previous();
    manager->setPosition(1);
    manager->setPosition("", 1);
    manager->singleService();
    manager->setSingleService(false);
    manager->currentService();
    manager->setCurrentService("");
    manager->availableServices();
    manager->canQuit();
    manager->canRaise();
    manager->canSetFullscreen();
    manager->desktopEntry();
    manager->fullscreen();
    manager->setFullscreen(false);
    manager->hasTrackList();
    manager->identity();
    manager->supportedUriSchemes();
    manager->supportedMimeTypes();
    manager->canControl();
    manager->canGoNext();
    manager->canGoPrevious();
    manager->canPause();
    manager->canPlay();
    manager->canSeek();
    manager->loopStatus();
    manager->maximumRate();
    manager->metadata();
    manager->minimumRate();
    manager->playbackStatus();
    manager->position();
    manager->requestPosition();
    manager->rate();
    manager->setRate(1);
    manager->shuffle();
    manager->setShuffle(false);
    manager->volume();
    manager->setVolume(50);
    manager->quit();
    manager->stop();
    ASSERT_FALSE(manager->quit());
}

TEST(MprisController, control)
{
    QDBusConnection con = QDBusConnection::sessionBus();
    MprisController *control = new MprisController("", con);
    control->isValid();
    control->quit();
    control->raise();
    control->next();
    QUrl url("file:/usr/share/music/bensound-sunny.mp3");
    control->openUri(url);
    control->pause();
    control->play();
    control->playPause();
    control->previous();
    control->seek(1);
    control->setPosition(1);
    control->stop();
    control->service();
    control->canQuit();
    control->canRaise();
    control->canSetFullscreen();
    control->desktopEntry();
    control->fullscreen();
    control->setFullscreen(false);
    control->hasTrackList();
    control->identity();
    control->supportedUriSchemes();
    control->supportedMimeTypes();
    control->canControl();
    control->canGoNext();
    control->canGoPrevious();
    control->canPause();
    control->canPlay();
    control->canSeek();
    control->loopStatus();
    control->maximumRate();
    control->metadata();
    control->minimumRate();
    control->playbackStatus();
    control->position();
    control->requestPosition();
    control->rate();
    control->setRate(1);
    control->shuffle();
    control->setShuffle(false);
    control->volume();
    control->setVolume(1);
//    ASSERT_EQ(control->volume(), 1);
}

TEST(MprisRootInterface, rootinterface)
{
    QDBusConnection con = QDBusConnection::sessionBus();
    MprisRootInterface *interface = new MprisRootInterface(" ", " ", con) ;
    interface->canQuit();
    interface->canRaise();
    interface->canSetFullscreen();
    interface->desktopEntry();
    interface->fullscreen();
    interface->setFullscreen(false);
    interface->hasTrackList();
    interface->identity();
    interface->supportedMimeTypes();
    interface->supportedUriSchemes();
    interface->Raise();
    interface->Quit();

    MprisPlayerInterface *playinter = new MprisPlayerInterface(" ", " ", con);
    playinter->staticInterfaceName();
    playinter->canControl();
    playinter->canGoNext();
    playinter->canGoPrevious();

    playinter->canPause();
    playinter->canPlay();
    playinter->canSeek();
    playinter->loopStatus();
    playinter->setLoopStatus("");
    playinter->maximumRate();
    playinter->metadata();
    playinter->minimumRate();
    playinter->playbackStatus();

    playinter->position();
    playinter->rate();
    playinter->setRate(1);
    playinter->shuffle();

    playinter->setShuffle(false);
    playinter->volume();
    playinter->setVolume(false);
    playinter->Next();
    playinter->OpenUri("");

    playinter->Pause();
    playinter->Play();
    playinter->PlayPause();
    playinter->Previous();
    playinter->Seek(1);
//    playinter->SetPosition();
    playinter->Stop();
}

TEST(MprisPlayer, player)
{
    MprisPlayer *player = new MprisPlayer;
    player->serviceName();
    player->setServiceName("");
    player->canQuit();
    player->setCanQuit(false);

    player->canRaise();
    player->setCanRaise(false);
    player->canSetFullscreen();
    player->setCanSetFullscreen(false);
    player->desktopEntry();
    player->setDesktopEntry("");
    player->fullscreen();
    player->setFullscreen(false);
    player->hasTrackList();
    player->setHasTrackList(false);
    player->identity();
    player->setIdentity("");
    player->supportedUriSchemes();
    player->supportedMimeTypes();
    player->canControl();
    player->setCanControl(false);
    player->canGoNext();
    player->setCanGoNext(false);
    player->canGoPrevious();
    player->setCanGoPrevious(false);
    player->canPause();
    player->setCanPause(false);
    player->canPlay();
    player->setCanPlay(false);
    player->canSeek();
    player->setCanSeek(false);
    player->loopStatus();
    player->maximumRate();
    player->setMaximumRate(1);
    player->metadata();
    player->minimumRate();
    player->setMinimumRate(1);
    player->playbackStatus();
    player->position();
    player->setPosition(1);
    player->rate();
    player->setRate(1);
    player->shuffle();
    player->setShuffle(false);
    player->volume();
    player->setVolume(1);
}

TEST(DBusUtils, utils)
{
    DBusUtils *dbus = new DBusUtils;
    Q_UNUSED(dbus)
}

TEST(Footer, footer)
{
    Footer *foot = new Footer;
    foot->setLyricButtonChecked(true);
    foot->setPlaylistButtonChecked(true);
    foot->showPlayListWidget(1, 1, true);
    foot->setSize(1, 1, true);
    foot->refreshBackground();
    foot->hidewaveform();
    foot->toggleLyricView();
//    QTest::qWait(200);
}
