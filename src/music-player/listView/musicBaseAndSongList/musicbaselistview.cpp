// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QMargins delegateMargins = delegate->margins();
    delegateMargins.setLeft(18);
    delegate->setMargins(delegateMargins);
    setItemDelegate(delegate);

    setViewportMargins(8, 0, 8, 0);
    auto font = this->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    setFont(font);

#ifdef DTKWIDGET_CLASS_DSizeMode
    slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
#else
    setIconSize(QSize(20, 20));
    setItemSize(QSize(40, 40));
#endif

    setFrameShape(QFrame::NoFrame);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setColor(DPalette::ItemBackground, Qt::transparent);
    DApplicationHelper::instance()->setPalette(this, pa);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::CustomContextMenu);

    init();

    connect(this, &MusicBaseListView::customContextMenuRequested, this, &MusicBaseListView::showContextMenu);
    connect(this, &MusicBaseListView::clicked, this, &MusicBaseListView::slotItemClicked);
    connect(Player::getInstance(), &Player::signalUpdatePlayingIcon, this, &MusicBaseListView::slotUpdatePlayingIcon);
    connect(CommonService::getInstance(), &CommonService::signalSwitchToView, this, &MusicBaseListView::viewChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MusicBaseListView::slotUpdatePlayingIcon);
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &MusicBaseListView::slotSizeModeChanged);
#endif
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
    DStandardItem *item = new DStandardItem(QIcon::fromTheme("music_album"), displayName);
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

    setThemeType(DGuiApplicationHelper::instance()->themeType());
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
    menu.addAction(tr("Add music"));

    menu.exec(globalPos);
}

void MusicBaseListView::setThemeType(int type)
{
    for (int i = 0; i < model->rowCount(); i++) {
        auto curStandardItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(model->index(i, 0)));
        curStandardItem->setForeground(type == 1 ? QColor("#414D68") : QColor("#C0C6D4"));
    }
}

void MusicBaseListView::dragEnterEvent(QDragEnterEvent *event)
{
    auto t_formats = event->mimeData()->formats();
    qDebug() << t_formats;
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("playlistview/x-datalist")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void MusicBaseListView::dragMoveEvent(QDragMoveEvent *event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && (event->mimeData()->hasFormat("text/uri-list")  || event->mimeData()->hasFormat("playlistview/x-datalist"))) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->acceptProposedAction();
        DListView::dragMoveEvent(event);
    }
}

void MusicBaseListView::dropEvent(QDropEvent *event)
{
    QModelIndex indexDrop = indexAt(event->pos());
    if (!indexDrop.isValid())
        return;
    QString hash = indexDrop.data(Qt::UserRole + 2).value<QString>();

    if ((!event->mimeData()->hasFormat("text/uri-list") && !event->mimeData()->hasFormat("playlistview/x-datalist"))) {
        return;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        auto urls = event->mimeData()->urls();
        QStringList localpaths;
        for (auto &url : urls) {
            localpaths << (url.isLocalFile() ? url.toLocalFile() : url.path());
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

//    DListView::dropEvent(event);
}

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

        QSize iconSize(20, 20);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        iconSize.setWidth(16);
        iconSize.setHeight(16);
    }
#endif
        QString hash = index.data(Qt::UserRole + 2).value<QString>();
        if (hash == Player::getInstance()->getCurrentPlayListHash()) {
            QPixmap playingPixmap = QPixmap(iconSize);
            playingPixmap.fill(Qt::transparent);
            QPainter painter(&playingPixmap);
            DTK_NAMESPACE::Gui::DPalette pa;// = this->palette();
            if (selectedIndexes().size() > 0 && (selectedIndexes().at(0) == index)) {
                painter.setPen(QColor(Qt::white));
            } else {
                painter.setPen(pa.color(QPalette::Active, DTK_NAMESPACE::Gui::DPalette::Highlight));
            }
            Player::getInstance()->playingIcon().paint(&painter, QRect(0, 0, iconSize.width(), iconSize.height()), Qt::AlignCenter, QIcon::Active, QIcon::On);

            QIcon playingIcon(playingPixmap);
            DViewItemActionList actionList = item->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            } else {
                actionList.clear();
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter, iconSize);
                viewItemAction->setParent(this);
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
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter, iconSize);
                //初始化指明父类，方便后续释放，防止内存泄露
                viewItemAction->setParent(this);
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
    } else if (action->text() == tr("Add music")) {
        emit CommonService::getInstance()->signalAddMusic();
    }  else if (action->text() == tr("Pause")) {
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

#ifdef DTKWIDGET_CLASS_DSizeMode
void MusicBaseListView::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        setIconSize(QSize(16, 16));
        setItemSize(QSize(24, 24));
    } else {
        setIconSize(QSize(20, 20));
        setItemSize(QSize(40, 40));
    }
}
#endif
