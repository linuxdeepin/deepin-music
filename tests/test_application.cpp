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
#include "inotifyfiles.h"
#include "lyric.h"
#include "musiclyric.h"
#include "widget/waveform.h"

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
#include "dbusextendedabstractinterface.h"
#include "playlistmanager.h"
#include "widget/musiclistview.h"
#include "widget/searchmetaitem.h"

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
    a->adjustSize();
    a->update();
    ASSERT_EQ(10, a->volume());
}

TEST(SearchEdit, isNull)
{
    SearchEdit *a = new  SearchEdit() ;
    SearchResult *b = new SearchResult;
    a->setResultWidget(b);
    PlaylistPtr playl;
    //b->onSearchCand("a", playl);
    a->onFocusOut();
    a->onTextChanged();
    a->onReturnPressed();
    a->curPlaylistPtr().isNull();
    //ASSERT_TRUE(isNull);
}

TEST(Cover, setcolor)
{
    Cover *a = new  Cover() ;
    QPixmap pix;
    QColor color(255, 255, 255);
    a->radius();
    a->setCoverPixmap(pix);
    a->setRadius(1);
    ASSERT_EQ(1, a->radius());
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
    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget wid;
    a->paint(&painter, option, index);
    a->createEditor(&wid, option, index);
    a->setEditorData(&wid, index);
//    delete a;
//    a = nullptr;
}

TEST(MusicInfoItemDelegate, isRunning7)
{
    MusicInfoItemDelegate *a = new  MusicInfoItemDelegate() ;
//    QPainter painter;
    QStyleOptionViewItem option;
    option.widget = new QWidget;
    QModelIndex index;
    QWidget wid;
    a->createEditor(&wid, option, index);
    a->setEditorData(&wid, index);
}

TEST(MusicListDataDelegate, isRunning8)
{
    MusicListDataDelegate *a = new  MusicListDataDelegate() ;
    QWidget *parent = new QWidget;
    QPainter painter;
    QStyleOptionViewItem option;
    option.widget = new QWidget;
    QModelIndex index;
//    a->paint(&painter, option, index);
    a->initStyleOption(&option, index);
    a->createEditor(parent, option, index);
    a->setEditorData(parent, index);
//    a->sizeHint(option, index);
    ASSERT_TRUE(a != nullptr);
}

TEST(MusicInfoItemDelegatePrivate, setcolor)
{
    MusicInfoItemDelegatePrivate *a = new  MusicInfoItemDelegatePrivate() ;
    QStyleOptionViewItem option;
    a->textColor();
    a->titleColor();
    a->highlightText();
    a->background();
    a->playingIcon();
    QColor color(255, 255, 255);
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

extern int tailPointWidth(const QStyleOptionViewItem &option);
TEST(PlayItemDelegate, isRunning12)
{
    PlayItemDelegate *a = new  PlayItemDelegate() ;
    QStyleOptionViewItem option;
    tailPointWidth(option);
    option.widget = new QWidget;
    QModelIndex index;
    QWidget wid;
    a->createEditor(&wid, option, index);
    a->setEditorData(&wid, index);
    Q_UNUSED(a)
}

TEST(PlayItemDelegatePrivate, setcolor)
{
    PlayItemDelegatePrivate *a = new  PlayItemDelegatePrivate() ;
    a->textColor();
    a->titleColor();
    a->highlightText();
    a->background();
    a->playingIcon();
    QColor color(255, 255, 255);
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
    QWidget *w = new QWidget;
    w->setFixedSize(500, 600);
    LyricLabel *a = new  LyricLabel(true, w) ;
    a->getFromFile("123.lrc");
    QPainter *p = new QPainter(a);
    QRect rt(QPoint(20, 20), QPoint(40, 40));
    //a->paintEvent(nullptr);
    a->setCurrentIndex(2);
    a->currentIndex();
    //a->paintItem(p, 0, rt);
//    a->paintItem(p, 1, rt);
    a->event(new QScrollPrepareEvent(QPoint(20, 20)));
    a->event(new QScrollEvent(QPoint(20, 20), QPoint(24, 20), QScrollEvent::ScrollStarted));
    a->event(new QWheelEvent(QPointF(10, 30), 1,
                             Qt::RightButton, Qt::ControlModifier));
    a->event(new QWheelEvent(QPointF(10, 30), 125,
                             Qt::RightButton, Qt::ControlModifier));
    a->event(new QEvent(QEvent::MouseButtonPress));
    a->event(new QEvent(QEvent::None));
    int height = a->itemHeight();
    ASSERT_TRUE(height > 0);
}

TEST(LyricView, viewMode)
{
    LyricView *a = new  LyricView() ;
    QWheelEvent event(a->pos(), 1, Qt::MouseButtons(Qt::MouseButtonMask), Qt::KeyboardModifier::NoModifier);
    QApplication::sendEvent(a, &event);
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
    a->flags(QModelIndex());
    ASSERT_TRUE(a->playlist() == nullptr);
}

TEST(MusiclistInfomodel, isRunning23)
{
    MusiclistInfomodel *a = new  MusiclistInfomodel() ;
    PlaylistPtr playlist;
    a->setPlaylist(playlist);
    a->playlist();
    a->flags(QModelIndex());
}

TEST(MusiclistModel, isRunning24)
{
    MusiclistModel *a = new  MusiclistModel() ;
    PlaylistPtr playlist;
    a->flags(QModelIndex());
    Q_UNUSED(a)
}

TEST(MusicSearchListModel, isRunning25)
{
    MusicSearchListModel *a = new  MusicSearchListModel() ;
    a->setPlaylist(PlaylistPtr());
    a->playlist();
    a->flags(QModelIndex());
}

extern void margeDatabase();
TEST(PlaylistModel, findmusic)
{
    auto metalist = MediaDatabase::instance()->allmetas();
    margeDatabase();
    PlaylistMeta all;
    MediaDatabase::instance()->addPlaylist(all);
    MediaDatabase::instance()->updatePlaylist(all);
    MediaDatabase::instance()->removePlaylist(all);
    MediaDatabase::instance()->deleteMusic(globMetaPtr, all);
    MediaDatabase::instance()->playlistExist("all");

    MediaMeta *meta = new MediaMeta();
    if (metalist.isEmpty()) {
        meta->hash = "music";
    } else
        *meta = metalist.at(0);
    PlaylistModel *a = new  PlaylistModel() ;
    PlaylistModel *b = new  PlaylistModel(1, 1, nullptr) ;
    Q_UNUSED(b);
    QModelIndex index;
    a->meta(index);
    a->playlist();

    //datastream
    QDataStream steam;
    steam << (static_cast<void>(QDataStream()), MetaPtr(meta));
}

TEST(MusicBoxButton, boxbutton)
{
    MusicBoxButton *a = new  MusicBoxButton("") ;
    MusicBoxButton *b = new  MusicBoxButton("", "", "", "", "") ;
    a->setTransparent(true);
    a->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");
    a->setPropertyPic("", 1, ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");
    b->setAutoChecked(true);
    b->setPropertyPic(":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/normal/play_normal.svg",
                      ":/mpimage/light/press/play_press.svg");
    a->show();
    QPoint point = a->pos();
    QTestEventList e;
    e.addMouseMove(point + QPoint(5, 5), 10);
    e.addMouseMove(point + QPoint(6, 6), 10);
    e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, point, 10);
    e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, point, 10);
    e.addMouseMove(point + QPoint(100, 100), 10);
    e.simulate(a);
    ASSERT_TRUE(a != nullptr);
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
    a->show();
    QPoint point = a->pos();
    QTestEventList e;
    e.addMouseMove(point + QPoint(5, 5), 10);
    e.addMouseMove(point + QPoint(6, 6), 10);
    e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, point, 10);
    e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, point, 10);
    e.addMouseMove(point + QPoint(100, 100), 10);
    e.simulate(a);
    ASSERT_TRUE(a != nullptr);

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
    a->show();
    QPoint point = a->pos();
    QTestEventList e;
    e.addMouseMove(point + QPoint(5, 5), 10);
    e.addMouseMove(point + QPoint(6, 6), 10);
    e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, point, 10);
    e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, point, 10);
    e.addMouseMove(point + QPoint(100, 100), 10);
    e.simulate(a);
    ASSERT_TRUE(a != nullptr);
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
    QTestEventList e;
    e.addKeyPress(Qt::Key_A, Qt::NoModifier);
    e.addKeyPress(Qt::Key_A, Qt::ShiftModifier);
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier);
    e.simulate(a);
    ASSERT_TRUE(a != nullptr);
}

TEST(MusicListView, listview)
{
    MusicListView *view = new MusicListView;
    view->item(0, 0);
    view->showContextMenu(QPoint(0, 0));
    view->clearSelected();
    view->adjustHeight();
    view->getSizeChangedFlag();
    view->setSizeChangedFlag(false);

    QPoint point = view->pos();
    QTestEventList e;
    e.addMouseMove(point + QPoint(1, 1), 10);
    e.addMouseMove(point + QPoint(2, 2), 10);
    e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, point + QPoint(2, 2), 10);
    e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, point + QPoint(2, 2), 10);
    e.addKeyPress(Qt::Key_A, Qt::NoModifier);
    e.addMouseMove(point + QPoint(100, 100), 10);
    e.simulate(view);
    ASSERT_TRUE(view != nullptr);
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
    a->onFocusOut();
    a->onTextChanged();
    a->onReturnPressed();
    QTestEventList e;
    e.addKeyPress(Qt::Key_Up, Qt::NoModifier);
    e.addKeyPress(Qt::Key_Down, Qt::NoModifier);
    e.simulate(a);
    ASSERT_TRUE(a != nullptr);
}

TEST(SearchLyricsWidget, isRunning38)
{
    lyric stru;
    SearchLyricsWidget *a = new  SearchLyricsWidget("") ;
    a->setSearchDir("/home/zhangyong/Music/");
    a->setDefault("a.mp3", "a");
    a->setThemeType(1);
    a->setThemeType(0);
    a->searchLyrics();
}

TEST(SearchMetaItem, smitem)
{
    SearchMetaItem *item = new SearchMetaItem;
    DMusic::SearchMeta meta;
    meta.id = "music";
    DMusic::SearchArtist art;
    art.name = "1";
    meta.artists.append(art);
    item->initUI(meta);
    item->setChecked(false);
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
    b->autoResize();
    b->setSearchString("");
    b->selectUp();
    b->selectDown();
    b->currentStr();
    b->onReturnPressed();
    b->slotTheme(0);
    b->getSearchStr();
    b->clearKeyState();
}

TEST(ToolTips, isRunning40)
{
    ToolTips *a = new  ToolTips("") ;
    a->radius();
    a->borderColor();
    a->background();
    a->setText("");
    a->setRadius(1);
    QPoint p = a->pos();
//    a->pop(p);
}

TEST(WaveformScale, isRunning41)
{
    WaveformScale *a = new  WaveformScale();
    a->show();
    a->setValue(1);

    a->move(0, 0);
    a->adjustSize();
    a->update();

    a->hide();
    class  wfs : public WaveformScale
    {
    public:
        void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
            WaveformScale::paintEvent(event);
        }
    };

    wfs w;
    w.paintEvent(nullptr);
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
    a->onProgressChanged(1, 1);
    PlaylistMeta playlist;
    PlaylistPtr playlist1(new Playlist(playlist));
    a->onMusicPlayed(playlist1, globMetaPtr);
    a->onMusicStop(playlist1, globMetaPtr);
    DMusic::SearchMeta sea;
    DMusic::SearchMeta sea1("1");
    QString lyr = "123.lrc";
    a->onLyricChanged(globMetaPtr, sea, lyr.toUtf8());
    a->onCoverChanged(globMetaPtr, sea, lyr.toUtf8());
    QColor color(255, 255, 255, 0);
    a->onUpdateMetaCodec(globMetaPtr);
    a->setGeometry(10, 10, 100, 100);
    a->adjustSize();
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
    MusicLyricWidget *a = new  MusicLyricWidget() ;
    a->updateUI();
    a->defaultCover();
    a->slotTheme(1);
    a->onProgressChanged(1, 1);
    a->onsearchBt();
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
    meta_->cuePath = "/usr/share/music/bensound-sunny.mp3";
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QFileInfo fileinfo(path);
    metadet->updateMetaFromLocalfile(meta.data(), fileinfo);
    metadet->getCoverData(path, path, meta->hash);
    metadet->updateCueFileTagCodec(meta_, fileinfo, "utf8");
    metadet->updateMediaFileTagCodec(meta_, "utf8", false);
    metadet->updateMediaFileTagCodec(meta_, "GB2312", false);
    metadet->detectEncodings(meta);
//    QVector<float> vec = metadet->getMetaData(path);
//    ASSERT_FALSE(vec.isEmpty());
}

TEST(BaseTool, basetool)
{
    QString xml = "123.xml";
//    QString path = "/usr/share/music/bensound-sunny.mp3";
    QString lrc = "123.lrc";
    QFile fin(lrc);
    if (fin.open(QIODevice::ReadOnly)) {
        QByteArray data = fin.readAll();
        fin.close();
        QByteArray encoding = BaseTool::detectEncode(data, lrc);//歌词
        encoding = BaseTool::detectEncode(data, xml);
        ASSERT_TRUE(encoding.size() > 0);
    }
}

extern qint64 timeframe2mtime(long frame);
//extern void Libcue::time_frame_to_msf(long frame, int *m, int *s, int *f);
TEST(CueParser, cue)
{
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QString path2 = "/usr/share/sounds/alsa/Noise.wav";

    DMusic::CueParser *cue = new DMusic::CueParser(path, "UTF-8");
    DMusic::CueParser *cue2 = new DMusic::CueParser(path2, "");
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
//    QTestEventList e;
//    e.addKeyPress(Qt::Key_Enter, Qt::NoModifier);
    ASSERT_FALSE(v);
}

TEST(MetaBufferDetector, metabuf)
{
    QString path = "/usr/share/music/bensound-sunny.mp3";
    QString hash = "z";
    MetaBufferDetector *buf = new MetaBufferDetector;
    buf->onBufferDetector(path, hash);
    buf->onClearBufferDetector();
    buf->start();
    QTest::qWait(30);
}

extern QString cacheCoverPath(const MetaPtr meta);
extern QString cacheCoverPath(const QString &searchID);
TEST(MetaSearchService, metaservice)
{
    MusicSettings::init();
    MusicSettings::value("volume");
    MetaSearchService   *lyricService = MetaSearchService::instance();
//    cacheLyricPath(globMetaPtr);
    cacheCoverPath(globMetaPtr);
//    cacheLyricPth("");
    //cacheCoverPath("");

    PluginManager *plu = new PluginManager;
    plu->init();

    lyricService->init();
    lyricService->searchContext("");
    lyricService->coverData(globMetaPtr);
    lyricService->lyricData(globMetaPtr);
    lyricService->coverUrl(globMetaPtr);
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
//    PlaylistPtr playlist;
//    Player::instance()->playPrevMusic(playlist, globMetaPtr);
    ASSERT_FALSE(Player::instance() == nullptr);
}

extern const MetaPtrList shuffle(MetaPtrList &&musiclist, int seed);
extern bool moreThanTimestamp(const MetaPtr v1, const MetaPtr v2);
extern bool moreThanTitle(const MetaPtr v1, const MetaPtr v2);
extern bool moreThanArtist(const MetaPtr v1, const MetaPtr v2);
extern bool moreThanAblum(const MetaPtr v1, const MetaPtr v2);
TEST(Playlist, playlist)
{
    globMetaPtr->hash = "all";
    PlaylistMeta meta;
    meta.uuid = "all";
    meta.displayName = "music";
    meta.sortID = 1;
    MediaMeta *meta2 = new MediaMeta();
    meta2->hash = "music";
    meta2->length = 1;
    MetaPtr meta1(meta2);
    Playlist *list = new Playlist(meta);
    list->load();
    list->updateMeta(globMetaPtr);
    list->prev(globMetaPtr);
    list->prev(meta1);
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
    list->shuffleNext(meta1);
    list->shufflePrev(globMetaPtr);
    list->shufflePrev(meta1);
    list->isLast(globMetaPtr);
    list->reset(list->allmusic());
    list->appendMusicList(list->allmusic());
    list->removeMusicList(list->allmusic());
    list->removeOneMusic(globMetaPtr);
    list->removeOneMusic(meta1);
    list->resort();
//    PlayMusicTypePtr  musicTypePtr;
//    list->appendMusicTypePtrListData(musicTypePtr);
    QMap<QString, int > index;
    index.insert("", 0);
    list->saveSort(index);
    QStringList listsortMeta;
    listsortMeta << "music";
    list->playMusicTypeToMeta("index", listsortMeta);
    moreThanTimestamp(meta1, meta1);
    moreThanTitle(meta1, meta1);
    moreThanArtist(meta1, meta1);
    moreThanAblum(meta1, meta1);
    ASSERT_EQ(0, list->viewMode());
}

TEST(PlaylistManager, manager)
{
    PlaylistManager *ma = new PlaylistManager;
    ma->newID();
    ma->newDisplayName();
    ma->allplaylist();
    ma->playlist("");
//    QStringList list;
//    list << "musicddffffffff";
//    ma->onCustomResort(list);
    ma->load();
    ma->saveSortOrder();
}

TEST(PluginManager, plugin)
{
    PluginManager *plu = new PluginManager;
    plu->init();
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

    QString lyc = "[00:03.71]";
    Lyric ly = parseLrc(lyc);
    QString path = "123.lrc";
    ly = parseLrc(path);
    ASSERT_FALSE(ly.hasTime);
}

TEST(ThreadPool, thread)
{
    ThreadPool *th = new ThreadPool;
    QThread *q = th->newThread();
    QLabel *lab = new QLabel;
    th->moveToNewThread(lab);
//    th->manager(q);
    th->quitAll();
}

TEST(MusicLyric, mly)
{
    MusicLyric *mly = new MusicLyric;
    QString path = "123.lrc";
    mly->getFromFile(path);
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
    instance->status();
    instance->logLevel();
    VlcMediaPlayer *player = new VlcMediaPlayer(instance);
    player->equalizer()->setPreamplification(12);
    player->equalizer();
    player->audio();
    player->length();
    VlcMedia *vmedia = new VlcMedia("/usr/share/music/bensound-sunny.mp3", true, instance);
    vmedia->initMedia("/usr/share/music/bensound-sunny.mp3", true, instance);
    QString path = "/usr/share/music/";
    QString name = "bensound-sunny.mp3";
    Vlc::Mux m = Vlc::TS;
    Vlc::AudioCodec ac = Vlc::MP3;
    Vlc::VideoCodec vc = Vlc::NoVideo;
    vmedia->record(name, path, m, ac, vc, false);
    vmedia->record(name, path, m, false);
    vmedia->record(name, path, m, ac, vc, 1, 1, 1, false);
    vmedia->setOption("mp3");

    VlcAudio *audio = new VlcAudio(player);
    audio->setVolume(50);
    audio->toggleMute();
    audio->setMute(true);
    audio->track();
    audio->getMute();
    audio->volume();

    player->play();

    QTest::qWait(200);
    VolumeMonitoring vot(nullptr);
    vot.needSyncLocalFlag();
    vot.syncLocalFlag(1);
    vot.syncLocalFlag(0);
    vot.start();
    vot.readSinkInputValid();
    vot.timeoutSlot();
    vot.stop();

    player->pause();
    player->setTime(10000);
    player->resume();
    player->core();
    player->state();
    player->setPosition(5000);
    player->position();
    vmedia->core();
    vmedia->state();
    vmedia->duplicate("ssssstx", "/usr/share/music/bensound-sunny.mp3", Vlc::TS);
    ASSERT_TRUE(audio->getMute());
}

TEST(VLC, vlc)
{
    Vlc *vlc = new Vlc;
    vlc->audioCodec();

    vlc->mux();
    vlc->ratio();
    vlc->scale();
    vlc->videoCodec();
    vlc->mux();
    vlc->ratio();
    vlc->scale();
    vlc->videoCodec();
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
    widget2->hide();
    widget1->hide();
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
    QUrl url("/usr/share/music/bensound-sunny.mp3");
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
    dbus->readDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio", "com.deepin.daemon.Audio", "SinkInputs");
    Q_UNUSED(dbus)
}

TEST(Footer, footer)
{
    Footer *foot = new Footer;
    foot->setLyricButtonChecked(true);
    foot->setPlaylistButtonChecked(true);
    foot->showPlayListWidget(1, 1, true);
    foot->refreshBackground();
    foot->hidewaveform();
    foot->toggleLyricView();
    foot->adjustSize();
    foot->update();
//    QTest::qWait(200);
}

TEST(Waveform, waf)
{
    Waveform wafm(Qt::Horizontal, new QWidget(), nullptr);
    QByteArray  ba("0100000010101111000010101111010101010110");
    //wafm.onAudioBufferProbed(new QAudioBuffer());

}

