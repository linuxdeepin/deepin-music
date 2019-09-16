#ifndef SEARCHLYRICSWIDGET_H
#define SEARCHLYRICSWIDGET_H

#include <DFloatingWidget>
#include <DPalette>
#include <DPushButton>
#include <DListWidget>
#include <DLineEdit>

#include <QWidget>
#include <QVector>
#include <QButtonGroup>

#include "mediameta.h"

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

struct lyric {
    QString lyricsName;
    QString singerName;
    QString duration;
    QString path;
    lyric()
    {
        lyricsName = "null";
        singerName = "null";
        duration = "null";
    }
};

class SearchLyricsWidget : public DFloatingWidget
{
    Q_OBJECT

public:
    SearchLyricsWidget(QString path, QWidget *parent = nullptr);

    void setSearchDir(QString dir)
    {
        m_path = dir;
    }
    void setDefault(QString song, QString singer);

    void setThemeType(int type);

public slots:
    void searchLyrics();

private slots:
    void mySelection(QModelIndex index);
    void mySelectionIndex(int index);
    void slottextChanged(const QString &str);
signals:
    //点击歌词列表item传出歌词路径
    void lyricPath(QString path);
    void checkBoxClicked(int index);
private:
    //第一个linedit歌曲名，第二个歌手名
    void initUI();
    void initConnection();
    void createList();

    DPushButton             *m_search;
    DListWidget             *m_listWidget;
    DLineEdit               *m_keyWord;
    DLineEdit               *m_singer;
    QString                  m_path;
    QButtonGroup            *m_group;
    QVector<lyric>           m_lyricList;
};

#endif // SEARCHLYRICSWIDGET_H
