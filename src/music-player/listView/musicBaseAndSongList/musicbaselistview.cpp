/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#include "musicbaselistview.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>

#include <DMenu>
#include <DDialog>
#include <DScrollBar>
#include <DPalette>
#include <DApplicationHelper>
#include <QDomDocument>
#include <QDomElement>
#include <QSvgRenderer>
#include <QPainter>

#include "databaseservice.h"
#include "commonservice.h"
#include "musicbaseandsonglistmodel.h"
#include "mediameta.h"
#include "player.h"
#include "databaseservice.h"
#include "playlistview.h"

DGUI_USE_NAMESPACE

MusicBaseListView::MusicBaseListView(QWidget *parent) : DListView(parent)
{
    model = new MusicBaseAndSonglistModel(this);
    setModel(model);
    delegate = new DStyledItemDelegate(this);
    if (CommonService::getInstance()->isTabletEnvironment()) {
        delegate->setBackgroundType(DStyledItemDelegate::NoBackground);
    }
    auto delegateMargins = delegate->margins();
    delegateMargins.setLeft(18);
    delegate->setMargins(delegateMargins);
    setItemDelegate(delegate);

    setViewportMargins(8, 0, 8, 0);
    auto font = this->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    setFont(font);

    setIconSize(QSize(20, 20));
    setItemSize(QSize(40, 40));

    setFrameShape(QFrame::NoFrame);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setColor(DPalette::ItemBackground, Qt::transparent);
    DApplicationHelper::instance()->setPalette(this, pa);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicBaseListView::customContextMenuRequested,
            this, &MusicBaseListView::showContextMenu);

//    connect(this, &MusicBaseListView::triggerEdit,
//    this, [ = ](const QModelIndex & index) {
//        if (DGuiApplicationHelper::instance()->themeType() == 1) {
//            auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(index));
//            curStandardItem->setIcon(QIcon(QString(":/mpimage/light/normal/famous_ballad_normal.svg")));
//        }
//    });
    init();
    connect(this, &MusicBaseListView::clicked,
            this, &MusicBaseListView::slotItemClicked);

    connect(Player::getInstance(), &Player::signalUpdatePlayingIcon,
            this, &MusicBaseListView::slotUpdatePlayingIcon);

    connect(CommonService::getInstance(), &CommonService::signalSwitchToView, this, &MusicBaseListView::viewChanged);


    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MusicBaseListView::slotUpdatePlayingIcon);
}

MusicBaseListView::~MusicBaseListView()
{

}

void SetSVGBackColor1(QDomElement elem, QString strtagname, QString strattr, QString strattrval)
{
    if (elem.tagName().compare(strtagname) == 0) {
        QString before_color = elem.attribute(strattr);
        elem.setAttribute(strattr, strattrval);
        QString color = elem.attribute(strattr);
    }
    for (int i = 0; i < elem.childNodes().count(); i++) {
        if (!elem.childNodes().at(i).isElement()) {
            continue;
        }
        SetSVGBackColor1(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
    }
}

void MusicBaseListView::init()
{
    QString displayName = tr("Albums");
    auto item = new DStandardItem(QIcon::fromTheme("music_album"), displayName);
    item->setData(ListPageSwitchType::AlbumType, Qt::UserRole);
    item->setData("album", Qt::UserRole + 2);
    model->appendRow(item);

    displayName = tr("Artists");
    item = new DStandardItem(QIcon::fromTheme("music_singer"), displayName);
    item->setData(ListPageSwitchType::SingerType, Qt::UserRole);
    item->setData("artist", Qt::UserRole + 2);
    model->appendRow(item);

    displayName = tr("All Music");
    item = new DStandardItem(QIcon::fromTheme("music_allmusic"), displayName);
    item->setData(ListPageSwitchType::AllSongListType, Qt::UserRole);
    item->setData("all", Qt::UserRole + 2);
    model->appendRow(item);

    displayName = tr("My Favorites");
    item = new DStandardItem(QIcon::fromTheme("music_mycollection"), displayName);
    item->setData(ListPageSwitchType::FavType, Qt::UserRole);
    item->setData("fav", Qt::UserRole + 2);
    model->appendRow(item);

    setMinimumHeight(model->rowCount() * 40);
}

void MusicBaseListView::showContextMenu(const QPoint &pos)
{
    auto index = indexAt(pos);
    if (!index.isValid())
        return;

    auto item = model->itemFromIndex(index);
    if (!item) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);


    DMenu menu;
    connect(&menu, &DMenu::triggered, this, &MusicBaseListView::slotMenuTriggered);

    QAction *playact = nullptr;
    QAction *pauseact = nullptr;

    ListPageSwitchType type = index.data(Qt::UserRole).value<ListPageSwitchType>();

    QString hash = index.data(Qt::UserRole + 2).value<QString>();
    emit CommonService::getInstance()->signalSwitchToView(type, "");

    if (hash == Player::getInstance()->getCurrentPlayListHash() && Player::getInstance()->status() == Player::Playing) {
        pauseact = menu.addAction(tr("Pause"));
        setActionDisabled(hash, pauseact);
    } else {
        playact = menu.addAction(tr("Play"));
        setActionDisabled(hash, playact);
    }

    menu.exec(globalPos);
}

//void MusicBaseListView::mousePressEvent(QMouseEvent *event)
//{
//    DListView::mousePressEvent(event);
//}

void MusicBaseListView::dragEnterEvent(QDragEnterEvent *event)
{
    auto t_formats = event->mimeData()->formats();
    qDebug() << t_formats;
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    }
}

void MusicBaseListView::dragMoveEvent(QDragMoveEvent *event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && (event->mimeData()->hasFormat("text/uri-list")  || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        DListView::dragMoveEvent(event);
    }
}

void MusicBaseListView::dropEvent(QDropEvent *event)
{
    QModelIndex indexDrop = indexAt(event->pos());
    if (!indexDrop.isValid())
        return;
    QString hash = indexDrop.data(Qt::UserRole + 2).value<QString>();

//    auto t_playlistPtr = playlistPtr(index);
    if (/*t_playlistPtr == nullptr || */(!event->mimeData()->hasFormat("text/uri-list") && !event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        return;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        auto urls = event->mimeData()->urls();
        QStringList localpaths;
        for (auto &url : urls) {
            localpaths << url.toLocalFile();
        }

        if (!localpaths.isEmpty()) {
            if (hash == "fav") {
                DataBaseService::getInstance()->importMedias(hash, localpaths);
            } else {
                DataBaseService::getInstance()->importMedias("all", localpaths);
            }
        }
    } else {
        auto *source = qobject_cast<PlayListView *>(event->source());
        if (source != nullptr) {
            QList<MediaMeta> metas;
            for (auto index : source->selectionModel()->selectedIndexes()) {
                MediaMeta imt = index.data(Qt::UserRole).value<MediaMeta>();
                if (imt.mmType != MIMETYPE_CDA)
                    metas.append(imt);
            }

            if (!metas.isEmpty()) {
                if (hash == "fav") {
                    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(hash, metas);
                    emit CommonService::getInstance()->signalFluashFavoriteBtnIcon();
                    emit CommonService::getInstance()->signalShowPopupMessage(model->itemFromIndex(indexDrop)->text(), metas.size(), insertCount);
                }
            }
        }
    }

    DListView::dropEvent(event);
}

//void MusicBaseListView::SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval)
//{
//    // if it has the tagname then overwritte desired attribute
//    if (elem.tagName().compare(strtagname) == 0) {
//        elem.setAttribute(strattr, strattrval);
//    }
//    // loop all children
//    for (int i = 0; i < elem.childNodes().count(); i++) {
//        if (!elem.childNodes().at(i).isElement()) {
//            continue;
//        }
//        this->SetAttrRecur(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
//    }
//}

void MusicBaseListView::setActionDisabled(const QString &hash, QAction *act)
{
    if (hash == "album") {
        act->setDisabled(0 == DataBaseService::getInstance()->allAlbumInfos().size());
    } else if (hash == "artist") {
        act->setDisabled(0 == DataBaseService::getInstance()->allSingerInfos().size());
    } else if (hash == "all") {
        act->setDisabled(0 == DataBaseService::getInstance()->allMusicInfos().size());
    } else if (hash == "fav") {
        act->setDisabled(0 == DataBaseService::getInstance()->customizeMusicInfos("fav").size());
    }
}

void MusicBaseListView::slotUpdatePlayingIcon()
{
    for (int i = 0; i < model->rowCount(); i++) {
        QModelIndex index = model->index(i, 0);
        DStandardItem *item = dynamic_cast<DStandardItem *>(model->item(i, 0));
        if (item == nullptr) {
            continue;
        }
        QString hash = index.data(Qt::UserRole + 2).value<QString>();
        if (hash == Player::getInstance()->getCurrentPlayListHash()) {
            QPixmap playingPixmap = QPixmap(QSize(20, 20));
            playingPixmap.fill(Qt::transparent);
            QPainter painter(&playingPixmap);
            DTK_NAMESPACE::Gui::DPalette pa;// = this->palette();
            if (selectedIndexes().size() > 0 && (selectedIndexes().at(0) == index)) {
                painter.setPen(QColor(Qt::white));
            } else {
                painter.setPen(pa.color(QPalette::Active, DTK_NAMESPACE::Gui::DPalette::Highlight));
            }
            Player::getInstance()->playingIcon().paint(&painter, QRect(0, 0, 20, 20), Qt::AlignCenter, QIcon::Active, QIcon::On);

            QIcon playingIcon(playingPixmap);
            DViewItemActionList actionList = item->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            } else {
                actionList.clear();
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter, QSize(20, 20));
                viewItemAction->setIcon(playingIcon);
                actionList.append(viewItemAction);
                dynamic_cast<DStandardItem *>(item)->setActionList(Qt::RightEdge, actionList);
            }
        } else {
            QIcon playingIcon;
            DViewItemActionList actionList = item->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            } else {
                actionList.clear();
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter, QSize(20, 20));
                viewItemAction->setIcon(playingIcon);
                actionList.append(viewItemAction);
                dynamic_cast<DStandardItem *>(item)->setActionList(Qt::RightEdge, actionList);
            }
        }
    }
    update();
}

void MusicBaseListView::slotMenuTriggered(QAction *action)
{
    if (action->text() == tr("Play")) {
        emit CommonService::getInstance()->signalPlayAllMusic();
    } else if (action->text() == tr("Pause")) {
        Player::getInstance()->pause();
    }
}

void MusicBaseListView::slotItemClicked(const QModelIndex &index)
{
    ListPageSwitchType type = index.data(Qt::UserRole).value<ListPageSwitchType>();
    emit CommonService::getInstance()->signalSwitchToView(type, "");
    // 选中后刷新播放动态图
    slotUpdatePlayingIcon();
}

void MusicBaseListView::viewChanged(ListPageSwitchType switchtype, QString hashOrSearchword)
{
    Q_UNUSED(hashOrSearchword)
    if (switchtype == AllSongListType) {
        for (int i = 0; i <  model->rowCount(); i++) {
            QModelIndex curIndex = model->index(i, 0);
            QString metaTemp = curIndex.data(Qt::UserRole + 2).value<QString>();
            if (metaTemp == "all") {
                this->setCurrentIndex(curIndex);
                break;
            }
        }
    }
    if (switchtype == CustomType || switchtype == CdaType) {
        this->clearSelection();
    }
}
