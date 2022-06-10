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

#include "musicsonglistview.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>
#include <QPushButton>
#include <QDomDocument>
#include <QDomElement>
#include <QSvgRenderer>
#include <QPainter>
#include <QShortcut>
#include <QUuid>
#include <QDrag>

#include <DMenu>
#include <DDialog>
#include <DScrollBar>
#include <DPalette>
#include <DApplicationHelper>
#include <DFloatingMessage>
#include <DMessageManager>
#include <DApplication>
#include <DBlurEffectWidget>

#include "mediameta.h"
#include "musicsettings.h"
#include "commonservice.h"
#include "musicbaseandsonglistmodel.h"
#include "databaseservice.h"
#include "player.h"
#include "playlistview.h"
#include "ac-desktop-define.h"
#include "databaseservice.h"

#define CDA_USER_ROLE "CdaRole"
#define CDA_USER_ROLE_OFFSET 12  //userrole+12 防止和其他歌单role重叠
#define DRAGICON_LEFTBORDER 0
#define DRAGICON_TOPBORDER 0
#define DRAGICON_ROUNDRADIUS 10

DGUI_USE_NAMESPACE


MusicItemDelegate::MusicItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

void MusicItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DStyledItemDelegate::paint(painter, option, index);
    MusicSongListView *view = static_cast<MusicSongListView *>(option.styleObject);
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    QPen pen(color, 1);
    painter->setPen(pen);
    if (view == nullptr) return;
    int curRowCount = view->m_model->rowCount();
    bool dragFlag = view->m_isDraging;
    QRect borderRect = option.rect;
    int hRow = view->highlightedRow();
    // 绘制拖拽分割线
    if (!view->selectedIndexes().contains(index) && dragFlag) {
        if (hRow == index.row()) {
            painter->drawLine(QLine(QPoint(borderRect.x(), borderRect.top() + 1), QPoint(borderRect.width(), borderRect.top() + 1)));
        } else if ((index.row() == (curRowCount - 1)) && (hRow == curRowCount || hRow == -1)) {
            painter->drawLine(QLine(QPoint(borderRect.x(), borderRect.bottom() - 1), QPoint(borderRect.width(), borderRect.bottom() - 1)));
        }
    }
}

MusicSongListView::MusicSongListView(QWidget *parent) : DListView(parent)
{
    this->setEditTriggers(NoEditTriggers);

    m_model = new MusicBaseAndSonglistModel(this);
    setModel(m_model);
    m_delegate = new MusicItemDelegate(this);
    if (CommonService::getInstance()->isTabletEnvironment()) {
        m_delegate->setBackgroundType(DStyledItemDelegate::NoBackground);
    }
    auto delegateMargins = m_delegate->margins();
    delegateMargins.setLeft(18);
    m_delegate->setMargins(delegateMargins);
    setItemDelegate(m_delegate);

    setViewportMargins(8, 0, 8, 0);
    auto font = this->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    setFont(font);

    setIconSize(QSize(ItemIconSide, ItemIconSide));
    setItemSize(QSize(ItemHeight, ItemHeight));

    setFrameShape(QFrame::NoFrame);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setColor(DPalette::ItemBackground, Qt::transparent);
    DApplicationHelper::instance()->setPalette(this, pa);

    setDropIndicatorShown(true);
    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);

    connect(&m_dragScrollTimer, SIGNAL(timeout()), this, SLOT(slotUpdateDragScroll()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicSongListView::customContextMenuRequested,
            this, &MusicSongListView::showContextMenu);

    init();
    initShortcut();
    connect(this, &MusicSongListView::clicked, this, [](QModelIndex midx) {
        qDebug() << "customize midx.row()" << midx.row();
        if (midx.row() == 0 && midx.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE)
            emit CommonService::getInstance()->signalSwitchToView(CdaType, midx.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString());
        else
            emit CommonService::getInstance()->signalSwitchToView(CustomType, midx.data(Qt::UserRole).toString());
    });

    connect(Player::getInstance(), &Player::signalUpdatePlayingIcon,
            this, &MusicSongListView::slotUpdatePlayingIcon);

    connect(this, &MusicSongListView::doubleClicked, this, &MusicSongListView::slotDoubleClicked);
    connect(this, &MusicSongListView::currentChanged, this, &MusicSongListView::slotCurrentChanged);

    connect(CommonService::getInstance(), &CommonService::signalAddNewSongList, this, &MusicSongListView::addNewSongList);
    //connect(CommonService::getInstance(), &CommonService::signalCdaSongListChanged, this, &MusicSongListView::changeCdaSongList);
    connect(CommonService::getInstance(), &CommonService::signalCdaSongListChanged, this, &MusicSongListView::slotPopMessageWindow);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MusicSongListView::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

MusicSongListView::~MusicSongListView()
{

}

void MusicSongListView::init()
{
    QList<DataBaseService::PlaylistData> list = DataBaseService::getInstance()->allPlaylistMeta();

    for (int i = 0; i < list.size(); i++) {
        DataBaseService::PlaylistData data = list.at(i);
        // 使用专门的属性判断是否自定义歌单
        if (data.readonly == 1) {
            continue;
        }
        QString displayName = data.displayName;
        DStandardItem *item = new DStandardItem(QIcon::fromTheme("music_famousballad"), displayName);

        item->setData(data.uuid, Qt::UserRole);
        item->setForeground(DGuiApplicationHelper::instance()->themeType() == 1 ? QColor("#414D68") : QColor("#C0C6D4"));
        m_model->appendRow(item);
    }
    setMinimumHeight(m_model->rowCount() * ItemHeight);
}

void MusicSongListView::showContextMenu(const QPoint &pos)
{
    auto index = indexAt(pos);
    if (!index.isValid()) return;

    auto item = m_model->itemFromIndex(index);
    if (!item || (index.row() == 0 && index.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE)) {
        return;//cda item不做任何处理
    }

    QPoint globalPos = this->mapToGlobal(pos);

    DMenu menu;
    connect(&menu, &DMenu::triggered, this, &MusicSongListView::slotMenuTriggered);
    QString hash = index.data(Qt::UserRole).value<QString>();
    emit CommonService::getInstance()->signalSwitchToView(ListPageSwitchType::CustomType, hash);

    if (!CommonService::getInstance()->isTabletEnvironment()) {
        QAction *playact = nullptr;
        QAction *pauseact = nullptr;

        if (hash == Player::getInstance()->getCurrentPlayListHash() && Player::getInstance()->status() == Player::Playing) {
            pauseact = menu.addAction(tr("Pause"));
            pauseact->setDisabled(0 == DataBaseService::getInstance()->customizeMusicInfos(hash).size());
        } else {
            playact = menu.addAction(tr("Play"));
            playact->setDisabled(0 == DataBaseService::getInstance()->customizeMusicInfos(hash).size());
        }
    }

    if (hash != "album" || hash != "artist" || hash != "all" || hash != "fav") {
        menu.addAction(tr("Add music"));
        menu.addAction(tr("Rename"));
        menu.addAction(tr("Delete"));
    }

    menu.exec(globalPos);
}

void MusicSongListView::adjustHeight()
{
    setMinimumHeight(m_model->rowCount() * ItemHeight);
}

bool MusicSongListView::getHeightChangeToMax()
{
    return m_heightChangeToMax;
}

void MusicSongListView::addNewSongList()
{
// 编辑功能逻辑变更，代码废弃
//    //close editor
//    for (int i = 0; i < model->rowCount(); i++) {
//        auto item = model->index(i, 0);
//        if (this->isPersistentEditorOpen(item))
//            closePersistentEditor(item);
//    }

    qDebug() << "new item";
    QIcon icon = QIcon::fromTheme("music_famousballad");

    QString displayName = newDisplayName();
    DStandardItem *item = new DStandardItem(icon, displayName);
    item->setForeground(DGuiApplicationHelper::instance()->themeType() == 1 ? QColor("#414D68") : QColor("#C0C6D4"));
    m_model->appendRow(item);
    setMinimumHeight(m_model->rowCount() * ItemHeight);
    setCurrentIndex(m_model->indexFromItem(item));
    //未显示时不编辑
    if (isVisible())
        slotDoubleClicked(m_model->indexFromItem(item));
    scrollToBottom();

    //record to db
    DataBaseService::PlaylistData info;
    info.editmode = true;
    info.readonly = false;
    info.uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    info.displayName = displayName;
    info.sortID = DataBaseService::getInstance()->getPlaylistMaxSortid();
    info.sortType = DataBaseService::SortByCustomASC;
    DataBaseService::getInstance()->addPlaylist(info);
    item->setData(info.uuid, Qt::UserRole); //covert to hash
    // 切换listpage
    emit CommonService::getInstance()->signalSwitchToView(CustomType, info.uuid);

    //主页面清空选择项
    emit sigAddNewSongList();
    m_heightChangeToMax = true;
    adjustHeight();
}

void MusicSongListView::changeCdaSongList(int stat)
{
    if (stat == 1) {
        //发送提示消息
        emit CommonService::getInstance()->signalShowPopupMessage("", 0, 0);
    }

    qDebug() << __FUNCTION__ << stat;
    if (m_model->rowCount() > 0) {
        QString rolestr = m_model->index(0, 0).data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString();
        if (rolestr == CDA_USER_ROLE) { //remove node
            if (stat != 1) {
                m_model->removeRow(0);  //移出CD歌单项目
            }
            return;
        }
    }
    if (stat != 1) {//CD弹出或其他异常
        return;
    }

    qDebug() << "new CD item";
    QIcon icon = QIcon::fromTheme("music_famousballad");

    QString displayName = tr("CD playlist");
    DStandardItem *item = new DStandardItem(icon, displayName);
    item->setForeground(DGuiApplicationHelper::instance()->themeType() == 1 ? QColor("#414D68") : QColor("#C0C6D4"));

    m_model->insertRow(0, item);
    setMinimumHeight(m_model->rowCount() * ItemHeight);
    setCurrentIndex(m_model->indexFromItem(item));
    scrollToTop();

    QString uuid = CDA_USER_ROLE;
    item->setData(uuid, Qt::UserRole + CDA_USER_ROLE_OFFSET);

    //主页面清空选择项
    emit sigAddNewSongList();
    m_heightChangeToMax = true;
    adjustHeight();
    emit CommonService::getInstance()->signalSwitchToView(CdaType, m_model->index(0, 0).data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString());
}

void MusicSongListView::rmvSongList()
{
    QModelIndex index = this->currentIndex();
    // 当前选中行
    int rowCurrent = index.row();
    // 总行数，删除后焦点定位使用
    int rowCount = m_model->rowCount();
    if (rowCurrent == 0 && index.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE) {
        return;//cda item不做任何处理
    }
    if (rowCurrent >= 0) {
        QString message = QString(tr("Are you sure you want to delete this playlist?"));

        DDialog warnDlg(this);
        warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
        warnDlg.setTextFormat(Qt::AutoText);
        warnDlg.setTitle(message);
        warnDlg.addSpacing(20);
        warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
        warnDlg.addButton(tr("Delete"), true, Dtk::Widget::DDialog::ButtonWarning);
        warnDlg.setObjectName(AC_MessageBox);
        if (1 == warnDlg.exec()) {
            QString hash = index.data(Qt::UserRole).value<QString>();
            bool isMediaMetaInSonglist = false;
            isMediaMetaInSonglist = DataBaseService::getInstance()->isMediaMetaInSonglist(hash, Player::getInstance()->getActiveMeta().hash);
            // 数据库删除歌单
            if (DataBaseService::getInstance()->deletePlaylist(hash)) {
                // 判断当前是否播放歌曲，如果正在播放，则判断当前歌曲是否在此歌单
                if (!Player::getInstance()->getActiveMeta().hash.isEmpty() && isMediaMetaInSonglist && Player::getInstance()->getCurrentPlayListHash() == hash) {
                    Player::getInstance()->clearPlayList();
                    // 设置所有音乐播放状态
                    Player::getInstance()->setCurrentPlayListHash("", false);
                }
                if (rowCount == 1) {
                    // 没有播放且只有一个歌单,删除后焦点定位到所有音乐
                    this->setCurrentIndex(QModelIndex());
                    emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "all");
                    this->clearSelection();
                } else if (rowCurrent < (rowCount - 1)) {
                    // 删除后焦点定位到下一个歌单
                    QModelIndex nextIndex = m_model->index((rowCurrent + 1), 0);
                    QString nextHash = nextIndex.data(Qt::UserRole).value<QString>();
                    this->setCurrentIndex(nextIndex);
                    // 显示CD列表
                    if (nextIndex.row() == 0 && nextIndex.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE)
                        emit CommonService::getInstance()->signalSwitchToView(CdaType, nextIndex.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString());
                    else
                        emit CommonService::getInstance()->signalSwitchToView(CustomType, nextHash);
                } else {
                    // 没有后一个歌单,删除后焦点定位到上一个歌单
                    QModelIndex nextIndex = m_model->index((rowCurrent - 1), 0);
                    QString nextHash = nextIndex.data(Qt::UserRole).value<QString>();
                    this->setCurrentIndex(nextIndex);
                    // 显示CD列表
                    if (nextIndex.row() == 0 && nextIndex.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE)
                        emit CommonService::getInstance()->signalSwitchToView(CdaType, nextIndex.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString());
                    else
                        emit CommonService::getInstance()->signalSwitchToView(CustomType, nextHash);
                }
                // 删除成功，移除歌单
                m_model->removeRow(rowCurrent);
            }
        }
        //删除消息，让scroll自动刷新
        m_heightChangeToMax = false;
        adjustHeight();
    }
}

void MusicSongListView::slotUpdatePlayingIcon()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DStandardItem *item = dynamic_cast<DStandardItem *>(m_model->item(i, 0));
        if (item == nullptr) {
            continue;
        }
        QString hash = index.data(Qt::UserRole).value<QString>();
        //获取cda的hash值，用于处理cd歌单的波浪条
        QString cdahash = index.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).value<QString>();
        if (!cdahash.isEmpty())
            hash = cdahash;
        if (hash == Player::getInstance()->getCurrentPlayListHash()) {
            QPixmap playingPixmap = QPixmap(ItemIconSide, ItemIconSide);
            playingPixmap.fill(Qt::transparent);
            QPainter painter(&playingPixmap);
            DTK_NAMESPACE::Gui::DPalette pa;// = this->palette();
            if (selectedIndexes().size() > 0 && (selectedIndexes().at(0) == index)) {
                painter.setPen(QColor(Qt::white));
            } else {
                painter.setPen(pa.color(QPalette::Active, DTK_NAMESPACE::Gui::DPalette::Highlight));
            }
            Player::getInstance()->playingIcon().paint(&painter, QRect(0, 0, ItemIconSide, ItemIconSide), Qt::AlignCenter, QIcon::Active, QIcon::On);

            QIcon playingIcon(playingPixmap);
            DViewItemActionList actionList = item->actionList(Qt::RightEdge);
            if (!actionList.isEmpty()) {
                actionList.first()->setIcon(playingIcon);
            } else {
                actionList.clear();
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter, QSize(ItemIconSide, ItemIconSide));
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
                auto viewItemAction = new DViewItemAction(Qt::AlignCenter, QSize(ItemIconSide, ItemIconSide));
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

void MusicSongListView::slotMenuTriggered(QAction *action)
{
    QModelIndex index = this->currentIndex();
    if (!index.isValid())
        return;

    if (action->text() == tr("Play")) {
        emit CommonService::getInstance()->signalPlayAllMusic();
    } else if (action->text() == tr("Add music")) {
        emit CommonService::getInstance()->signalAddMusic();
    } else if (action->text() == tr("Pause")) {
        Player::getInstance()->pause();
    } else if (action->text() == tr("Rename")) {
        slotDoubleClicked(index);
    } else if (action->text() == tr("Delete")) {
        rmvSongList();
    }
}

void MusicSongListView::slotCurrentChanged(const QModelIndex &cur, const QModelIndex &pre)
{
// 逻辑变更，目前不用这段代码
//    m_renameItem = dynamic_cast<DStandardItem *>(model->itemFromIndex(cur));
//    if (m_renameItem) {
//        m_renameItem->setIcon(QIcon::fromTheme("music_famousballad"));
//    }
    Q_UNUSED(cur)
    DStandardItem *preStandardItem = dynamic_cast<DStandardItem *>(m_model->itemFromIndex(pre));
    if (preStandardItem) {
        preStandardItem->setIcon(QIcon::fromTheme("music_famousballad"));
    }
}

void MusicSongListView::slotDoubleClicked(const QModelIndex &index)
{
    m_renameItem = dynamic_cast<DStandardItem *>(m_model->itemFromIndex(index));

    if (index.row() == 0 && index.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE) {
        return;//cda item不做任何处理
    }
    if (!m_renameItem) return;

    if (m_renameLineEdit == nullptr) {
        initRenameLineEdit();
    }
    m_renameLineEdit->setVisible(true);
    m_renameLineEdit->move(50, m_renameItem->row() * this->sizeHintForIndex(index).height() + ItemEditMargin);
    if (CommonService::getInstance()->isTabletEnvironment()) {
        QPoint pos = this->mapToGlobal(m_renameLineEdit->pos());
        CommonService::getInstance()->setCurrentWidgetPosY(pos.y());
    }
    m_renameLineEdit->setVisible(true);
    m_renameLineEdit->lineEdit()->setText(m_renameItem->text());
    m_renameLineEdit->lineEdit()->selectAll();
    m_renameLineEdit->lineEdit()->setFocus();
    if (CommonService::getInstance()->isTabletEnvironment() && !DApplication::inputMethod()->isVisible()) {
        DApplication::inputMethod()->show();
    }
}

void MusicSongListView::slotLineEditingFinished()
{
    if (m_renameLineEdit && m_renameLineEdit->isVisible()) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            m_renameLineEdit->clearFocus();
        }
        m_renameLineEdit->setVisible(false);

        if (!m_renameItem)
            return;

        m_renameItem->setText(m_renameLineEdit->text());
        QString uuid = m_renameItem->data(Qt::UserRole).value<QString>();
        // 歌单名去重
        for (int i = 0; i < m_model->rowCount() ; ++i) {
            DStandardItem *tmpItem = dynamic_cast<DStandardItem *>(m_model->itemFromIndex(m_model->index(i, 0)));
            if (m_renameItem->text().isEmpty() || (m_renameItem->row() != tmpItem->row() && m_renameItem->text() == tmpItem->text())) {
                QList<DataBaseService::PlaylistData> plist = DataBaseService::getInstance()->getCustomSongList();
                for (DataBaseService::PlaylistData data : plist) {
                    if (uuid == data.uuid) {
                        m_renameItem->setText(data.displayName);// 还原歌单名
                    }
                }
                m_renameItem->setIcon(QIcon::fromTheme("music_famousballad"));
                return;
            }
        }

        DataBaseService::getInstance()->updatePlaylistDisplayName(m_renameItem->text(), uuid);
        m_renameItem->setIcon(QIcon::fromTheme("music_famousballad"));
        // 防止焦点设置到其他控件 bug:61769
        if (!CommonService::getInstance()->isTabletEnvironment()) {
            this->setFocus();
        }
    }
}

void MusicSongListView::slotPopMessageWindow(int stat)
{
    //初始cd的track索引
    Player::getInstance()->initCddTrack();

    MediaMeta tmpMeta = Player::getInstance()->getActiveMeta();
    //CD退出时若不关闭弹窗，再次接入cd后，此弹窗自动隐藏，列表数据刷新
    QList<QWidget *> oldMsgList = this->findChildren<QWidget *>("_d_message_cda_deepin_music");
    if (oldMsgList.size() > 0) {
        oldMsgList.first()->setProperty("cda", 1);
        oldMsgList.first()->close();
        oldMsgList.first()->deleteLater();
        //执行添加/移出CD歌单,cd弹出
        changeCdaSongList(stat);
        return;
    }

    //只处理弹出cd的弹窗
    if (stat == 1) {
        changeCdaSongList(stat);
        if (DataBaseService::getInstance()->allMusicInfosCount() == 0) {
            //进入主页面
            emit CommonService::getInstance()->signalCdaImportFinished();
        }
        return;
    }

    //停止音乐
    //Player::getInstance()->stop();

    int retexec = -1;
    int iprop = 0;
    QString popStrMsg = "";

    popStrMsg = tr("The CD has been removed");
    if (tmpMeta.mmType == MIMETYPE_CDA) {
        //停止音乐
        Player::getInstance()->stop();
        popStrMsg = tr("Play failed, as the CD has been removed");
    }

    Dtk::Widget::DDialog tipsDlg(this);
    tipsDlg.setObjectName("_d_message_cda_deepin_music");
    tipsDlg.setIcon(QIcon::fromTheme("deepin-music"));
    tipsDlg.setTextFormat(Qt::RichText);
    tipsDlg.addButton(tr("OK"), true, Dtk::Widget::DDialog::ButtonNormal);
    tipsDlg.setMessage(popStrMsg);
    retexec = tipsDlg.exec();
    iprop = tipsDlg.property("cda").toInt();
    /**
     * 确保关闭前一个窗口后，再执行下一次的操作
     */
    qDebug() << __FUNCTION__ << "prop:" << iprop << "exec" << retexec;
    if (retexec != 0 && iprop == 1) {
        qDebug() << "________tipsDlg_______property" << retexec;
        return;
    }

    /**
     * 查看所有歌单是否有歌曲，无歌曲跳转到初始页面，有歌曲则跳转到所有页面播放第一首歌曲
     * */
    qDebug() << __FUNCTION__ << stat;
    int allsize = DataBaseService::getInstance()->allMusicInfosCount();
    if (allsize == 0) {
        //清空列表
        Player::getInstance()->clearPlayList();
        //回到初始页面
        emit DataBaseService::getInstance()->signalAllMusicCleared();
    } else {
        if (tmpMeta.mmType == MIMETYPE_CDA) {
            Player::getInstance()->setActiveMeta(MediaMeta());
            //设置当前页面，刷新播放队列和显示列表
            Player::getInstance()->setCurrentPlayListHash("all", true);
            //清空列表
            Player::getInstance()->clearPlayList();
            //播放歌单第一首歌曲
//            Player::getInstance()->forcePlayMeta();
        }

        if (m_model->rowCount() > 0  && tmpMeta.mmType != MIMETYPE_CDA) {
            int row = this->currentIndex().row();
            QString strrole = m_model->index(row, 0).data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString();
            if (strrole == CDA_USER_ROLE) {
                //设置当前页面，刷新播放队列和显示列表
                Player::getInstance()->setCurrentPlayListHash("all", false);
            }
        }
        // 切换到所有音乐界面
        emit CommonService::getInstance()->signalSwitchToView(AllSongListType, "all");
    }

    //执行添加/移出CD歌单,cd弹出
    changeCdaSongList(stat);
    //清空选中
    this->clearSelection();
}

void MusicSongListView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    if (m_renameLineEdit) {
        m_renameLineEdit->resize(this->width() - 64, ItemHeight - ItemEditMargin * 2);
    }
}

void MusicSongListView::keyReleaseEvent(QKeyEvent *event)
{
    // 重命名时不删除操作
    if (event->key() == Qt::Key_Delete && (m_renameLineEdit == nullptr || !m_renameLineEdit->isVisible())) {
        rmvSongList();
    }
    DListView::keyReleaseEvent(event);
}

QPixmap MusicSongListView::dragItemPixmap()
{
    qreal scale = devicePixelRatio();
    QModelIndexList modelIndexList = selectedIndexes();
    if (modelIndexList.size() != 1) return QPixmap();
    auto curSelectedItem = static_cast<DStandardItem *>(m_model->itemFromIndex(modelIndexList.first()));
    if (curSelectedItem->data(Qt::UserRole + 12).toString() == "CdaRole") return QPixmap();

    QString text = curSelectedItem->text();

    QFont font;
    font.setPixelSize(12);
    QFontMetrics fontMetrics(font);
    text = fontMetrics.elidedText(text, Qt::ElideRight, 100);
    int textWidth = fontMetrics.width(text) + DRAGICON_ROUNDRADIUS;
    int textHeight = fontMetrics.height() + 4;

    DBlurEffectWidget forwardWidget;
    forwardWidget.setBlurRectXRadius(DRAGICON_ROUNDRADIUS);
    forwardWidget.setBlurRectYRadius(DRAGICON_ROUNDRADIUS);
    forwardWidget.setRadius(30);
    forwardWidget.setBlurEnabled(true);
    forwardWidget.setMaskColor(QColor(247, 247, 247, 154));
    forwardWidget.setMaskAlpha(255);
    forwardWidget.setMode(DBlurEffectWidget::GaussianBlur);
    forwardWidget.setFixedSize(textWidth, textHeight);

    QPixmap forwardPixmap = forwardWidget.grab();

    QRect pixRect(0, 0, textWidth + DRAGICON_LEFTBORDER, textHeight + DRAGICON_TOPBORDER);
    QPixmap pixmap(pixRect.size() * scale);
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRect textRect(DRAGICON_LEFTBORDER, DRAGICON_TOPBORDER, textWidth, textHeight);

    painter.drawPixmap(textRect, forwardPixmap, QRect(0, 0, forwardPixmap.width(), forwardPixmap.height()));

    painter.save();
    painter.setFont(font);
    painter.setBrush(Qt::black);
    painter.drawText(textRect, text, QTextOption(Qt::AlignCenter));
    painter.restore();

    return pixmap;
}

void MusicSongListView::startDrag(Qt::DropActions supportedActions)
{
    emit clicked(indexAt(mapFromGlobal(QCursor::pos())));

    slotLineEditingFinished();
    QModelIndexList modelIndexList = selectedIndexes();
    if (modelIndexList.size() != 1) return;
    auto curSelectedItem = static_cast<DStandardItem *>(m_model->itemFromIndex(modelIndexList.first()));
    if (curSelectedItem->data(Qt::UserRole + 12).toString() == "CdaRole") return;

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << modelIndexList.first().row();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("MusicSongListView/x-datalist", itemData);

    QDrag *drag = new QDrag(this);
    drag->setPixmap(dragItemPixmap());
    drag->setMimeData(mimeData);
    Qt::DropAction dropAction = Qt::MoveAction;
    drag->exec(supportedActions, dropAction);
}

void MusicSongListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() != this && (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("playlistview/x-datalist"))) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();

        m_dragScrollTimer.start(200);
        m_isDraging = true;
        QModelIndex indexDrop = m_model->index(highlightedRow(), 0);
        update(indexDrop);
        m_preIndex = indexDrop;
    }
}

void MusicSongListView::slotUpdateDragScroll()
{
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    // 防止出边界
    if (!rect().contains(pos)) return;
    emit sigUpdateDragScroll();
}

void MusicSongListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() != this && (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("playlistview/x-datalist"))) {
        auto index = indexAt(event->pos());
        if (index.isValid()) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        } else {
            event->ignore();
        }
    } else if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();

        int curRow = highlightedRow();
        if (curRow == -1) curRow = m_model->rowCount() - 1;
        QModelIndex indexDrop = m_model->index(curRow, 0);
        //刷新旧区域使dropIndicator消失
        update(m_preIndex);
        update(indexDrop);
        m_preIndex = indexDrop;
    }
}

void MusicSongListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_dragScrollTimer.stop();
    m_isDraging = false;
    DListView::dragLeaveEvent(event);
}

int MusicSongListView::highlightedRow() const
{
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    QModelIndex curRowIndex = indexAt(pos);
    int curRow = curRowIndex.row();
    if (curRow != -1) {
        auto curIndexRect = rectForIndex(curRowIndex);
        if (pos.y() + verticalOffset() < curIndexRect.center().y()) {
            // 防止添加到cd之前
            if (curRow == 0 && curRowIndex.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE) {
                curRow += 1;
            }
        } else {
            curRow += 1;
        }
    }

    return curRow;
}

void MusicSongListView::dropItem(int preRow)
{
    auto curSelectedItem = static_cast<DStandardItem *>(m_model->itemFromIndex(m_model->index(preRow, 0)));
    if (curSelectedItem == nullptr)
        return;
    auto uuid = curSelectedItem->data(Qt::UserRole).toString();
    auto curText = curSelectedItem->text();

    int curRow = highlightedRow();
    // 防止重复添加
    if (curRow == preRow) {
        return;
    }
    if (curRow == -1) curRow = m_model->rowCount();
    // 删除前一个后需要将索引减去1
    if (curRow > preRow) curRow--;
    m_model->removeRow(preRow);
    QIcon icon = QIcon::fromTheme("music_famousballad");
    DStandardItem *item = new DStandardItem(icon, curText);
    item->setForeground(DGuiApplicationHelper::instance()->themeType() == 1 ? QColor("#414D68") : QColor("#C0C6D4"));
    m_model->insertRow(curRow, item);
    item->setData(uuid, Qt::UserRole);
    setCurrentIndex(m_model->index(curRow, 0));

    // 更新数据库
    QVector<QString> allHashs;
    for (int i = 0; i < m_model->rowCount(); i++) {
        auto curIndex = m_model->index(i, 0);
        if (i != 0 || curIndex.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() != CDA_USER_ROLE) {
            allHashs.append(curIndex.data(Qt::UserRole).toString());
        }
    }
    DataBaseService::getInstance()->sortAllPlaylist(allHashs);
}

void MusicSongListView::dropEvent(QDropEvent *event)
{
    m_isDraging = false;
    m_dragScrollTimer.stop();
    // 歌单处理
    if (event->source() == this) {
        auto curMimeData = event->mimeData();
        // 防止cd
        if (curMimeData == nullptr)
            return;

        QByteArray itemData(curMimeData->data("MusicSongListView/x-datalist"));
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        int preRow;
        dataStream >> preRow;
        dropItem(preRow);
    } else {
        QModelIndex indexDrop = indexAt(event->pos());
        if (!indexDrop.isValid())
            return;
        QString hash = indexDrop.data(Qt::UserRole).value<QString>();

        //cda item不做任何处理
        if ((!event->mimeData()->hasFormat("text/uri-list") && !event->mimeData()->hasFormat("playlistview/x-datalist")) || (indexDrop.data(Qt::UserRole + CDA_USER_ROLE_OFFSET).toString() == CDA_USER_ROLE)) {
            return;
        }

        if (event->mimeData()->hasFormat("text/uri-list")) {
            auto urls = event->mimeData()->urls();
            QStringList localpaths;
            for (auto &url : urls) {
                localpaths << (url.isLocalFile() ? url.toLocalFile() : url.path());
            }

            if (!localpaths.isEmpty()) {
                DataBaseService::getInstance()->importMedias(hash, localpaths);
            }
        } else {
            PlayListView *source = qobject_cast<PlayListView *>(event->source());
            if (source != nullptr) {
                QList<MediaMeta> metas;
                for (auto index : source->selectionModel()->selectedIndexes()) {
                    MediaMeta imt = index.data(Qt::UserRole).value<MediaMeta>();
                    if (imt.mmType != MIMETYPE_CDA)
                        metas.append(imt);
                }

                if (!metas.isEmpty()) {
                    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(hash, metas);
                    CommonService::getInstance()->signalShowPopupMessage(m_model->itemFromIndex(indexDrop)->text(), metas.size(), insertCount);
                }
            }
        }

        DListView::dropEvent(event);
    }
}

void MusicSongListView::initRenameLineEdit()
{
    m_renameLineEdit = new DLineEdit(this);
    // 初始化编辑控件大小
    m_renameLineEdit->resize(this->width() - ItemWidthDiff, ItemHeight - ItemEditMargin * 2);
    m_renameLineEdit->setVisible(false);
    m_renameLineEdit->setClearButtonEnabled(false);

    connect(m_renameLineEdit, &DLineEdit::editingFinished, this, &MusicSongListView::slotLineEditingFinished);
}

void MusicSongListView::slotRenameShortcut()
{
    if (selectedIndexes().size() > 0 && (!m_renameLineEdit || !m_renameLineEdit->isVisible())) {
        slotDoubleClicked(currentIndex());
    }
}

void MusicSongListView::slotEscShortcut()
{
    if (m_renameLineEdit && m_renameLineEdit->isVisible()) {
        m_renameLineEdit->setVisible(false);
    }
}

void MusicSongListView::initShortcut()
{
    // 移动到mainframe中统一管理
//    m_newItemShortcut = new QShortcut(this);
//    m_newItemShortcut->setContext(Qt::WidgetWithChildrenShortcut);
//    m_newItemShortcut->setKey(QKeySequence(QLatin1String("Ctrl+Shift+N")));
//    connect(m_newItemShortcut, &QShortcut::activated, this, &MusicSongListView::addNewSongList);

    m_renameShortcut = new QShortcut(QKeySequence(Qt::Key_F2), this);
    m_renameShortcut->setContext(Qt::WindowShortcut);
    connect(m_renameShortcut, &QShortcut::activated, this, &MusicSongListView::slotRenameShortcut);

    m_escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_escShortcut->setObjectName(AC_Shortcut_Escape);
    m_escShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(m_escShortcut, &QShortcut::activated, this, &MusicSongListView::slotEscShortcut);
}

//void MusicSongListView::setAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval)
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
//        this->setAttrRecur(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
//    }
//}

QString MusicSongListView::newDisplayName()
{
    QStringList existNames;
    for (DataBaseService::PlaylistData &playlistData : DataBaseService::getInstance()->allPlaylistMeta()) {
        existNames.append(playlistData.displayName);
    }

    QString temp = tr("New playlist");
    if (!existNames.contains(temp)) {
        return temp;
    }

    int i = 1;
    for (i = 1; i < existNames.size() + 1; ++i) {
        QString newName = QString("%1 %2").arg(temp).arg(i);
        if (!existNames.contains(newName)) {
            return newName;
        }
    }
    return QString("%1 %2").arg(temp).arg(i);
}

void MusicSongListView::setThemeType(int type)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        auto curStandardItem = dynamic_cast<DStandardItem *>(m_model->itemFromIndex(m_model->index(i, 0)));
        curStandardItem->setForeground(type == 1 ? QColor("#414D68") : QColor("#C0C6D4"));
    }
}

