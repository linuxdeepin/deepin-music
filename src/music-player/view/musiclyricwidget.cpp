/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "musiclyricwidget.h"

#include <QDebug>
#include <QFile>
#include <QScrollArea>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QStringListModel>
#include <QAbstractItemDelegate>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#include <DPalette>
#include <DPushButton>

#include "../core/util/lyric.h"
#include "../core/metasearchservice.h"

#include "widget/cover.h"
#include "widget/searchmetalist.h"
#include "widget/searchmetaitem.h"
#include "widget/searchlyricswidget.h"
#include "widget/lyriclabel.h"
#include "widget/musicimagebutton.h"

DGUI_USE_NAMESPACE

static const int LyricLineHeight = 40;
static const QString defaultLyric = "No Lyric";

class MUsicLyricWidgetPrivate
{
public:
    MUsicLyricWidgetPrivate(MUsicLyricWidget *parent): q_ptr(parent) {}

    MetaPtr             activingMeta;

    Cover               *m_cover              = nullptr;

    SearchLyricsWidget  *searchLyricsWidget   = nullptr;
    LyricLabel          *lyricview            = nullptr;

    MusicImageButton         *serachbt = nullptr;

    QString             defaultCover = ":/common/image/cover_max.svg";
    QColor              backgroundColor;

    bool               serachflag = false;

    MUsicLyricWidget *q_ptr;
    Q_DECLARE_PUBLIC(MUsicLyricWidget)
};

MUsicLyricWidget::MUsicLyricWidget(QWidget *parent)
    : DFrame(parent), d_ptr(new MUsicLyricWidgetPrivate(this))
{
    Q_D(MUsicLyricWidget);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    d->searchLyricsWidget = new SearchLyricsWidget(musicDir.first());
    d->searchLyricsWidget->hide();

    d->m_cover = new Cover;
    d->m_cover->setFixedSize(200, 200);
    d->m_cover->setObjectName("LyricCover");

    m_leftLayout = new QHBoxLayout(this);
    m_leftLayout->setContentsMargins(120, 190, 140, 160);
    m_leftLayout->addWidget(d->m_cover);
    m_leftLayout->addWidget(d->searchLyricsWidget);

    d->lyricview = new LyricLabel(false);

    auto searchlayout = new QVBoxLayout(this);
    d->serachbt = new MusicImageButton(":/mpimage/light/normal/search_normal.svg",
                                       ":/mpimage/light/normal/search_normal.svg",
                                       ":/mpimage/light/normal/search_normal.svg");
    d->serachbt->setProperty("typeName", true);
    d->serachbt->setPropertyPic("typeName", false, ":/mpimage/light/normal/back_normal.svg",
                                ":/mpimage/light/normal/back_normal.svg",
                                ":/mpimage/light/normal/back_normal.svg");
    d->serachbt->setFixedSize(48, 48);

    searchlayout->addWidget(d->serachbt);
    searchlayout->addStretch();
    searchlayout->setContentsMargins(58, 18, 34, 484);

    layout->addLayout(m_leftLayout, 4);
    layout->addWidget(d->lyricview, 4);
    layout->addLayout(searchlayout, 1);

    connect(d->serachbt, &DPushButton::clicked, this, &MUsicLyricWidget::onsearchBt);
    connect(d->searchLyricsWidget, &SearchLyricsWidget::lyricPath, this, &MUsicLyricWidget::slotonsearchresult);
}

MUsicLyricWidget::~MUsicLyricWidget()
{

}

void MUsicLyricWidget::updateUI()
{
    Q_D(MUsicLyricWidget);
    d->m_cover->setCoverPixmap(QPixmap(d->defaultCover));
}

QString MUsicLyricWidget::defaultCover() const
{
    Q_D(const MUsicLyricWidget);
    return d->defaultCover;
}

void MUsicLyricWidget::checkHiddenSearch(QPoint mousePos)
{
    Q_D(MUsicLyricWidget);

}

QColor MUsicLyricWidget::backgroundColor() const
{
    Q_D(const MUsicLyricWidget);
    return d->backgroundColor;
}


void MUsicLyricWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(MUsicLyricWidget);
    QWidget::resizeEvent(event);
}

void MUsicLyricWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(MUsicLyricWidget);
    if (d->serachflag && !d->searchLyricsWidget->rect().contains(event->pos())) {
        onsearchBt();
    }
    QWidget::mousePressEvent(event);
}

void MUsicLyricWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(MUsicLyricWidget);
    Q_UNUSED(playlist);
    d->activingMeta = meta;

    QFileInfo fileInfo(meta->localPath);
    QString lrcPath = fileInfo.dir().path() + QDir::separator() + fileInfo.completeBaseName() + ".lrc";
    d->lyricview->getFromFile(lrcPath);
    d->searchLyricsWidget->setSearchDir(fileInfo.dir().path() + QDir::separator());

    QImage cover(d->defaultCover);
    auto coverData = MetaSearchService::coverData(meta);
    if (coverData.length() > 0) {
        cover = QImage::fromData(coverData);
    }

    d->m_cover->setCoverPixmap(QPixmap::fromImage(cover));
    d->m_cover->update();
}

void MUsicLyricWidget::onMusicStop(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(MUsicLyricWidget);

    QImage cover(d->defaultCover);
    auto coverData = MetaSearchService::coverData(meta);
    if (coverData.length() > 0) {
        cover = QImage::fromData(coverData);
    }
    d->m_cover->setCoverPixmap(QPixmap::fromImage(cover));
    d->m_cover->update();
}

void MUsicLyricWidget::onProgressChanged(qint64 value, qint64 /*length*/)
{
    Q_D(MUsicLyricWidget);

    DPalette p = this->palette();
    p.setColor(DPalette::Background, d->backgroundColor);
    setPalette(p);

    d->lyricview->postionChanged(value);
}

void MUsicLyricWidget::onLyricChanged(const MetaPtr meta, const DMusic::SearchMeta &search,  const QByteArray &lyricData)
{
    Q_D(MUsicLyricWidget);
    if (d->activingMeta != meta) {
        return;
    }
}

void MUsicLyricWidget::onCoverChanged(const MetaPtr meta,  const DMusic::SearchMeta &search, const QByteArray &coverData)
{
    Q_D(MUsicLyricWidget);
    if (d->activingMeta != meta) {
        return;
    }

    QImage cover(d->defaultCover);
    if (coverData.length() > 0) {
        cover = QImage::fromData(coverData);
    }
    d->m_cover->setCoverPixmap(QPixmap::fromImage(cover));
    d->m_cover->update();
}

void MUsicLyricWidget::setDefaultCover(QString defaultCover)
{
    Q_D(MUsicLyricWidget);
    d->defaultCover = defaultCover;
}

void MUsicLyricWidget::onUpdateMetaCodec(const MetaPtr /*meta*/)
{
    Q_D(MUsicLyricWidget);

//    if (d->m_playingMusic == meta) {
//        d->m_playingMusic.title = meta.title;
//        d->m_playingMusic.artist = meta.artist;
//        d->m_playingMusic.album = meta.album;
    //    }
}

void MUsicLyricWidget::onsearchBt()
{
    Q_D(MUsicLyricWidget);
    d->serachflag = !d->serachflag;
    if (d->serachflag) {
        d->serachbt->setProperty("typeName", false);
        d->m_cover->hide();
        d->searchLyricsWidget->show();
        if (d->activingMeta != nullptr) {
            d->searchLyricsWidget->setDefault(d->activingMeta->title, d->activingMeta->artist);
        } else {
            d->searchLyricsWidget->setDefault("", "");
        }

        m_leftLayout->setContentsMargins(51, 21, 51, 19);
    } else {
        d->serachbt->setProperty("typeName", true);
        d->m_cover->show();
        d->searchLyricsWidget->hide();
        m_leftLayout->setContentsMargins(120, 190, 140, 160);
    }
}

void MUsicLyricWidget::slotonsearchresult(QString path)
{
    Q_D(MUsicLyricWidget);
    d->lyricview->getFromFile(path);
}

void MUsicLyricWidget::setBackgroundColor(QColor backgroundColor)
{
    Q_D(MUsicLyricWidget);
    d->backgroundColor = backgroundColor;
}

void MUsicLyricWidget::onContextSearchFinished(const QString &context, const QList<DMusic::SearchMeta> &metalist)
{
    Q_D(MUsicLyricWidget);

    //TODO: check context
    Q_UNUSED(context);
}


