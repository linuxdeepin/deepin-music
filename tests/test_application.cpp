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

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>
#include <DHiDPIHelper>

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
#include "../music-player/view/importwidget.h"
#include "../music-player/view/loadwidget.h"
#include "../music-player/view/lyricwidget.h"

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
}

TEST(SearchEdit, isRunning)
{
    SearchEdit *a = new  SearchEdit() ;
    Q_UNUSED(a)
}

TEST(Cover, isRunning2)
{
    Cover *a = new  Cover() ;
    ASSERT_EQ(8, a->radius());
}

TEST(Cover, isRunning4)
{
    InfoDialog *a = new  InfoDialog() ;
    Q_UNUSED(a)
}

TEST(DDropdown, isRunning5)
{
    DDropdown *a = new  DDropdown() ;
    Q_UNUSED(a)
}

TEST(LyricLineDelegate, isRunning6)
{
    LyricLineDelegate *a = new  LyricLineDelegate() ;
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
    Q_UNUSED(a)
}

TEST(MusicInfoItemDelegatePrivate, isRunning9)
{
    MusicInfoItemDelegatePrivate *a = new  MusicInfoItemDelegatePrivate() ;
    Q_UNUSED(a)
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
    Q_UNUSED(a)
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
    Q_UNUSED(a)
}

TEST(Label, isRunning17)
{
    Label *a = new  Label() ;
    Q_UNUSED(a)
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
    Q_UNUSED(a)
}

TEST(ModeButton, isRunning21)
{
    ModeButton *a = new  ModeButton() ;
    Q_UNUSED(a)
}

TEST(MusicListDataModel, isRunning22)
{
    MusicListDataModel *a = new  MusicListDataModel() ;
    Q_UNUSED(a)
}

TEST(MusiclistInfomodel, isRunning23)
{
    MusiclistInfomodel *a = new  MusiclistInfomodel() ;
    Q_UNUSED(a)
}

TEST(MusiclistModel, isRunning24)
{
    MusiclistModel *a = new  MusiclistModel() ;
    Q_UNUSED(a)
}

TEST(MusicSearchListModel, isRunning25)
{
    MusicSearchListModel *a = new  MusicSearchListModel() ;
    Q_UNUSED(a)
}

TEST(PlaylistModel, isRunning26)
{
    PlaylistModel *a = new  PlaylistModel() ;
    Q_UNUSED(a)
}

TEST(MusicBoxButton, isRunning27)
{
    MusicBoxButton *a = new  MusicBoxButton("") ;
    Q_UNUSED(a)
}

TEST(MusicIconButton, isRunning28)
{
    MusicIconButton *a = new  MusicIconButton() ;
    Q_UNUSED(a)
}

//TEST(MainFrame, isRunning29)
//{
//    MainFrame *a = new  MainFrame() ;
//    Q_UNUSED(a)
//}

TEST(MusicImageButton, isRunning30)
{
    MusicImageButton *a = new  MusicImageButton() ;
    Q_UNUSED(a)
}

TEST(MusicListDataView, isRunning31)
{
    MusicListDataView *a = new  MusicListDataView() ;
    Q_UNUSED(a)
}

TEST(MusicListDialog, isRunning32)
{
    MusicListDialog *a = new  MusicListDialog() ;
    Q_UNUSED(a)
}

TEST(MusicListInfoView, isRunning33)
{
    MusicListInfoView *a = new  MusicListInfoView() ;
    Q_UNUSED(a)
}

TEST(MusicSearchListview, isRunning34)
{
    MusicSearchListview *a = new  MusicSearchListview() ;
    Q_UNUSED(a)
}
TEST(ModelMake, isRunning35)
{
    ModelMake *a = new  ModelMake() ;
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
    Q_UNUSED(a)
}
TEST(SearchLyricsWidget, isRunning38)
{
    lyric stru;
    SearchLyricsWidget *a = new  SearchLyricsWidget("") ;
    Q_UNUSED(a)
}
TEST(TitlebarWidget, isRunning39)
{
    TitlebarWidget *a = new  TitlebarWidget() ;
    Q_UNUSED(a)
}

TEST(ToolTips, isRunning40)
{
    ToolTips *a = new  ToolTips("") ;
    Q_UNUSED(a)
}

TEST(WaveformScale, isRunning41)
{
    WaveformScale *a = new  WaveformScale() ;
    Q_UNUSED(a)
}

//TEST(Footer, isRunning42)
//{
//    Footer *a = new  Footer() ;
//    Q_UNUSED(a)
//}

TEST(ImportWidget, isRunning43)
{
    ImportWidget *a = new  ImportWidget() ;
    Q_UNUSED(a)
}

TEST(LoadWidget, isRunning44)
{
    LoadWidget *a = new  LoadWidget() ;
    Q_UNUSED(a)
}

TEST(LyricWidget, isRunning45)
{
    LyricWidget *a = new  LyricWidget() ;
    Q_UNUSED(a)
}
