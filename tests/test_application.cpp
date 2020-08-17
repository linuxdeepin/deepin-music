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
#include "../music-player/view/widget/soundvolume.h"
#include "../music-player/view/widget/searchedit.h"
#include "../music-player/view/widget/cover.h"
#include "../music-player/view/widget/infodialog.h"
#include "../music-player/view/widget/ddropdown.h"
#include "../music-player/view/widget/delegate/lyriclinedelegate.h"
#include "../music-player/view/widget/delegate/musicinfoitemdelegate.h"
#include "../music-player/view/widget/delegate/musiclistdatadelegate.h"
#include "../music-player/view/widget/delegate/musicinfoitemdelegate_p.h"
#include "../music-player/view/widget/delegate/musicitemdelegate_p.h"
#include "../music-player/view/widget/delegate/musicsearchlistdelegate.h"
#include "../music-player/view/widget/delegate/playitemdelegate.h"
#include "../music-player/view/widget/delegate/playitemdelegate_p.h"
#include "../music-player/view/widget/closeconfirmdialog.h"
#include "../music-player/view/widget/dequalizerdialog.h"
#include "../music-player/view/widget/filter.h"
#include "../music-player/view/widget/label.h"
#include "../music-player/view/widget/listview.h"
#include "../music-player/view/widget/lyriclabel.h"
#include "../music-player/view/widget/lyricview.h"
#include "../music-player/view/widget/modebuttom.h"
#include "../music-player/view/widget/searchresult.h"
#include "../music-player/view/widget/model/musiclistdatamodel.h"
#include "../music-player/view/widget/model/musiclistinfomodel.h"
#include "../music-player/view/widget/model/musiclistmodel.h"
#include "../music-player/view/widget/model/musicsearchlistmodel.h"
#include "../music-player/view/widget/model/playlistmodel.h"
#include "../music-player/view/widget/musicboxbutton.h"
#include "../music-player/view/widget/musiciconbutton.h"
#include "../music-player/view/widget/musicimagebutton.h"
#include "../music-player/view/widget/musiclistdataview.h"
#include "../music-player/view/widget/musiclistdialog.h"
#include "../music-player/view/widget/musiclistinfoview.h"
#include "../music-player/view/widget/musicsearchlistview.h"
#include "../music-player/view/widget/playlistview.h"
#include "../music-player/view/widget/pushbutton.h"
#include "../music-player/view/widget/searchedit.h"
#include "../music-player/view/widget/searchlyricswidget.h"
#include "../music-player/view/widget/titlebarwidget.h"
#include "../music-player/view/widget/tooltips.h"
#include "../music-player/view/widget/waveformscale.h"
//#include "../music-player/view/footerwidget.h"
#include "view/importwidget.h"
#include "view/loadwidget.h"
#include "view/lyricwidget.h"
#include "view/mainframe.h"
#include "core/AudioBufferDevice.h"
#include "view/shortcut.h"
#include "view/playlistwidget.h"
#include "view/musiclyricwidget.h"

DWIDGET_USE_NAMESPACE


int sum(int a, int b)
{
    return a + b;
}

TEST(qqq, www)
{
    ASSERT_EQ(2, sum(1, 1));
}

TEST(Application, isRunning3)
{
    SoundVolume *a = new  SoundVolume() ;
    ASSERT_EQ(20, a->radius());
    a->volume();
    a->setRadius(5);
    a->deleyHide();
    a->onVolumeChanged(10);
}

TEST(SearchEdit, isRunning)
{
    SearchEdit *a = new  SearchEdit() ;
    SearchResult *b = new SearchResult;
    a->setResultWidget(b);
    a->onFocusIn();
    a->onFocusOut();
    a->onTextChanged();
    a->onReturnPressed();
}

TEST(Cover, isRunning2)
{
    Cover *a = new  Cover() ;
    ASSERT_EQ(8, a->radius());
    QPixmap pix;
    a->radius();
    a->borderColor();
    a->shadowColor();
    a->setCoverPixmap(pix);
    a->setRadius(1);
    QColor color(255, 255, 255);
    a->setBorderColor(color);
    a->setShadowColor(color);
}

TEST(InfoDialog, isRunning4)
{
    InfoDialog *a = new  InfoDialog() ;
    MetaPtr meta ;
//    a->updateInfo(meta);
    a->setThemeType(0);
//    a->expand(true);
}

TEST(DDropdown, isRunning5)
{
    DDropdown *a = new  DDropdown() ;
    QString str = "aaa";
    a->status();
    a->actions();
    a->setText(str);
    a->setCurrentAction(0);
    a->setStatus(str);
}

TEST(LyricLineDelegate, isRunning6)
{
    LyricLineDelegate *a = new  LyricLineDelegate() ;
    delete a;
    a = nullptr;
    Q_UNUSED(a)
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
    a->createEditor(parent, option, index);
//    a->sizeHint(option, index);
}

TEST(MusicInfoItemDelegatePrivate, isRunning9)
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
}

TEST(MusicItemDelegatePrivate, isRunning10)
{
//    MusicItemDelegatePrivate *a = new  MusicItemDelegatePrivate() ;
//    Q_UNUSED(a)
}

TEST(MusicSearchListDelegate, isRunning11)
{
    MusicSearchListDelegate *a = new  MusicSearchListDelegate() ;
    Q_UNUSED(a)
}

TEST(PlayItemDelegate, isRunning12)
{
    PlayItemDelegate *a = new  PlayItemDelegate() ;
    Q_UNUSED(a)
}

TEST(PlayItemDelegatePrivate, isRunning13)
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
}

//TEST(CloseConfirmDialog, isRunning14)
//{
//    CloseConfirmDialog *a = new  CloseConfirmDialog() ;
//    Q_UNUSED(a)
//}

//TEST(DequalizerDialog, isRunning15)
//{
//    DequalizerDialog *a = new  DequalizerDialog() ;
//    Q_UNUSED(a)
//}

TEST(HoverFilter, isRunning16)
{
    HoverFilter *a = new  HoverFilter() ;
    HoverShadowFilter *b = new HoverShadowFilter();
    HintFilter *c = new HintFilter();
    c->hideAll();
    Q_UNUSED(a)
    Q_UNUSED(b)
}

TEST(Label, isRunning17)
{
    Label *a = new  Label() ;
    Label *b = new Label("asd", nullptr);
    Q_UNUSED(a)
    Q_UNUSED(b)
}

TEST(ListView, isRunning18)
{
    ListView *a = new  ListView() ;
    Q_UNUSED(a)
}

//TEST(LyricLabel, isRunning19)
//{
//    LyricLabel *a = new  LyricLabel(true) ;
//    Q_UNUSED(a)
//}

TEST(LyricView, isRunning20)
{
    LyricView *a = new  LyricView() ;
    a->viewMode();
    a->optical();
}

TEST(ModeButton, isRunning21)
{
    ModeButton *a = new  ModeButton() ;
    a->mode();
    a->setTransparent(true);
    a->setMode(1);
}

TEST(MusicListDataModel, isRunning22)
{
    MusicListDataModel *a = new  MusicListDataModel() ;
    a->playlist();
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

TEST(PlaylistModel, isRunning26)
{
    PlaylistModel *a = new  PlaylistModel() ;
    PlaylistModel *b = new  PlaylistModel(1, 1, nullptr) ;
    a->playlist();
}

TEST(MusicBoxButton, isRunning27)
{
    MusicBoxButton *a = new  MusicBoxButton("") ;
    MusicBoxButton *b = new  MusicBoxButton("", "", "", "", "") ;
    a->setTransparent(true);
    b->setAutoChecked(true);
}

TEST(MusicIconButton, isRunning28)
{
    MusicIconButton *a = new  MusicIconButton() ;
    Q_UNUSED(a)
}

//#include <QTest>

//TEST(MainFrame, isRunning29)
//{
//    setenv("PULSE_PROP_media.role", "music-test", 1);
//    MusicSettings::init();

//    int count = 0;
//    while (count++ < 20 && qApp == nullptr) {
//        QTest::qWait(100);
//    }

//    if (qApp == nullptr) {
//        QTest::qWarn("qApp is nullptr");
//        return;
//    }

//    MainFrame mainframe;
//    MusicApp *music = new MusicApp(&mainframe);
//    if (!music)
//        return;
//    music->initUI();
//}

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
    a->rowCount();
    a->listSize();
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
    WaveformScale *a = new  WaveformScale() ;
    a->setValue(1);
}

//TEST(Footer, isRunning42)
//{
//    Footer *a = new  Footer() ;
//    Q_UNUSED(a)
//}

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
}

TEST(AudioBufferDevice, isRunning46)
{
    QAudioFormat *format = new QAudioFormat();
    format->setSampleRate(44100);
    QAudioDecoder *decoder = new QAudioDecoder();
    decoder->setAudioFormat(*format);
    AudioBufferDevice *a = new  AudioBufferDevice(decoder) ;
    a->sliderReleased(1);
    a->onBufferReady();
    a->onFinished();
    a->clearQAbuffer();
}

TEST(Shortcut, isRunning47)
{
    MusicSettings::init();
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
