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

#include "importwidget.h"

#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStandardPaths>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <DFileDialog>
#include <DLabel>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include "musicsettings.h"
#include "databaseservice.h"
#include "player.h"
#include "ac-desktop-define.h"
#include "commonservice.h"
DGUI_USE_NAMESPACE

const QString linkTemplate = "<a href='%1'>%2</a>";
static constexpr int AnimationDelay = 200; //ms

ImportWidget::ImportWidget(QWidget *parent) : DFrame(parent)
{
    AC_SET_OBJECT_NAME(this, AC_ImportWidget);
    AC_SET_ACCESSIBLE_NAME(this, AC_ImportWidget);

    setAcceptDrops(true);
    setAutoFillBackground(true);

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_logo = new QLabel;
    m_logo->setFixedSize(128, 128);
    m_logo->setObjectName("ImportViewLogo");
    m_logo->setPixmap(QIcon::fromTheme("import_music_light").pixmap(QSize(128, 128)));

    m_waterProgress = new DWaterProgress(this);
    m_waterProgress->setTextVisible(true);
    m_waterProgress->setVisible(false);

    m_importPathButton = new DPushButton;
    auto pl = m_importPathButton->palette();
    pl.setColor(DPalette::Dark, QColor("#0098FF"));
    pl.setColor(DPalette::Light, QColor("#25B7FF"));
    pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0);
    pl.setColor(DPalette::Shadow, sbcolor);
//    d->importButton->setPalette(pl);
    m_importPathButton->setFixedSize(302, 36);
    m_importPathButton->setText(tr("Open Folder"));
    m_importPathButton->setFocusPolicy(Qt::TabFocus);
    m_importPathButton->setDefault(true);
    m_importPathButton->installEventFilter(this);
    DFontSizeManager::instance()->bind(m_importPathButton, DFontSizeManager::T6, QFont::Normal);

    m_addMusicButton = new DPushButton;
//    d->addMusicButton->setPalette(pl);
    m_addMusicButton->setFixedSize(302, 36);
    m_addMusicButton->setText(tr("Add Music"));
    m_addMusicButton->setFocusPolicy(Qt::TabFocus);
    m_addMusicButton->setDefault(true);
    m_addMusicButton->installEventFilter(this);
    DFontSizeManager::instance()->bind(m_addMusicButton, DFontSizeManager::T6, QFont::Normal);

    m_text = new QLabel;
    m_text->setObjectName("ImportViewText");
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    m_text->setText(tr("%1 music directory or drag music files here").arg(linkText));
    m_text->setFocusPolicy(Qt::TabFocus);
    m_text->installEventFilter(this);
    DFontSizeManager::instance()->bind(m_text, DFontSizeManager::T8, QFont::Normal);

    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_logo, 0, Qt::AlignCenter);
    layout->addWidget(m_waterProgress, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(m_addMusicButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_importPathButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_text, 0, Qt::AlignCenter);
    layout->addStretch();


    AC_SET_OBJECT_NAME(m_text, AC_importLinkText);
    AC_SET_ACCESSIBLE_NAME(m_text, AC_importLinkText);
    AC_SET_OBJECT_NAME(m_importPathButton, AC_importButton);
    AC_SET_ACCESSIBLE_NAME(m_importPathButton, AC_importButton);
    AC_SET_OBJECT_NAME(m_addMusicButton, AC_addMusicButton);
    AC_SET_ACCESSIBLE_NAME(m_addMusicButton, AC_addMusicButton);

    connect(m_addMusicButton, &DPushButton::clicked, this,  &ImportWidget::slotAddMusicButtonClicked);
    connect(m_importPathButton, &DPushButton::clicked, this,  &ImportWidget::slotImportPathButtonClicked);

    connect(m_text, &DLabel::linkActivated, this, &ImportWidget::slotLinkActivated);


    //connection
    connect(Player::getInstance()->getMpris(), &MprisPlayer::openUriRequested, this, &ImportWidget::slotImportFormDbus);  //open file form dbus


    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ImportWidget::setThemeType);

    connect(DataBaseService::getInstance(), &DataBaseService::signalImportedPercent,
            this, &ImportWidget::slotImportedPercent);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
}

ImportWidget::~ImportWidget()
{

}

void ImportWidget::closeAnimationToDown(const QSize &size)
{
    this->resize(size);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setDuration(AnimationDelay);
    animation->setStartValue(QPoint(0, 0));
    animation->setEndValue(QPoint(0, height()));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &ImportWidget::hide);

    animation->start();
}

void ImportWidget::showAnimationToLeft(const QSize &size)
{
    this->resize(size);
    this->show();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setDuration(AnimationDelay);
    animation->setStartValue(QPoint(size.width(), 0));
    animation->setEndValue(QPoint(0, 0));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    this->show();
    animation->start();
    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (CommonService::getInstance()->getSelectModel() == CommonService::MultSelect) {
            emit CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);
        }
    }
}

const QString ImportWidget::getLastImportPath() const
{
    QString lastImportPath = MusicSettings::value("base.play.last_import_path").toString();

    QDir lastImportDir = QDir(lastImportPath);
    if (!lastImportDir.exists() || lastImportPath.isEmpty()) {
        lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    } else {
        // blumia: always use the path from QDir, QDir also accept relative path string and it will pass
        //         the `QDir::exists()` checking
        lastImportPath = lastImportDir.absolutePath();
    }

    return lastImportPath;
}
// 添加需要导入的歌单hash
void ImportWidget::addMusic(QString listHash)
{
    if (listHash.isEmpty()) {
        listHash = "all";
    }
    DFileDialog fileDlg(this);
    QString lastImportPath = getLastImportPath();
    fileDlg.setDirectory(lastImportPath);
    QString selfilter = tr("All music") + (" (%1)");
    selfilter = selfilter.arg(Player::getInstance()->supportedSuffixList().join(" "));
    fileDlg.setViewMode(DFileDialog::Detail);
    fileDlg.setFileMode(DFileDialog::ExistingFiles);
    fileDlg.setOption(DFileDialog::HideNameFilterDetails);
    fileDlg.setNameFilter(selfilter);
    fileDlg.selectNameFilter(selfilter);
    fileDlg.setObjectName("fileDialogAdd");
    if (DFileDialog::Accepted == fileDlg.exec()) {
        showWaitHint();
        MusicSettings::setOption("base.play.last_import_path",  fileDlg.directory().path());
        DataBaseService::getInstance()->importMedias(listHash, fileDlg.selectedFiles());
    }
}

void ImportWidget::showImportHint()
{
    m_importPathButton->setDisabled(false);
    m_logo->setVisible(true);
    m_waterProgress->setVisible(false);
    m_waterProgress->stop();
    m_importPathButton->show();
    m_addMusicButton->show();
    QString linkText = QString(linkTemplate).arg(tr("Scan")).arg(tr("Scan"));
    m_text->setText(tr("%1 music directory or drag music files here").arg(linkText));
}

void ImportWidget::slotLinkActivated(const QString &link)
{
    Q_UNUSED(link)
    showWaitHint();
    QStringList musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    DataBaseService::getInstance()->importMedias("all", musicDir);
}

void ImportWidget::slotAddMusicButtonClicked()
{
    // 默认导入到所有音乐中
    addMusic("all");
}

void ImportWidget::slotImportPathButtonClicked()
{
    DFileDialog fileDlg(this);
    QString lastImportPath = getLastImportPath();
    fileDlg.setDirectory(lastImportPath);
    fileDlg.setViewMode(DFileDialog::Detail);
    fileDlg.setFileMode(DFileDialog::Directory);
    fileDlg.setObjectName("fileDialogImport");
    if (DFileDialog::Accepted == fileDlg.exec()) {
        showWaitHint();
        MusicSettings::setOption("base.play.last_import_path",  fileDlg.directory().path());
        DataBaseService::getInstance()->importMedias("all", fileDlg.selectedFiles());
    }
}

void ImportWidget::slotImportFormDbus(const QUrl &url)
{
    qDebug() << __FUNCTION__ << "toString = " << url.toString();
    qDebug() << __FUNCTION__ << "toLocalFile = " << url.toLocalFile();
    QString path = url.toLocalFile();
    if (path.isEmpty()) {
        path = url.toString().isEmpty() ? url.path() : url.toString(); //复杂名称可能出现tostring为空的问题，直接取path()
        if (path.isEmpty()) {
            return;
        }
    }
    DataBaseService::getInstance()->setFirstSong(path);
    DataBaseService::getInstance()->importMedias("all", QStringList() << path);
}

bool ImportWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_text) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(e);
            if (event->key() == Qt::Key_Return) {

                Q_EMIT this->scanMusicDirectory();

                auto pe =  m_text->palette();
                pe.setColor(QPalette::WindowText, QColor("#696969"));
                m_text->setPalette(pe);
            }
        } else if (e->type() == QEvent::FocusIn) {

            auto pe =  m_text->palette();
            pe.setColor(QPalette::WindowText, QColor("#A9A9A9"));
            m_text->setPalette(pe);

        } else if (e->type() == QEvent::FocusOut) {

            auto pe =  m_text->palette();
            pe.setColor(QPalette::WindowText, QColor("#696969"));
            m_text->setPalette(pe);
        }
    }

    return QWidget::eventFilter(o, e);
}


void ImportWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }

    DFrame::dragEnterEvent(event);
}

void ImportWidget::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

    if (!localpaths.isEmpty()) {
        DataBaseService::getInstance()->importMedias("all", localpaths);
        showWaitHint();
    }
}
void ImportWidget::setThemeType(int type)
{
    if (type == 1) {
        auto pl = m_importPathButton->palette();
        pl.setColor(DPalette::Dark, QColor("#0098FF"));
        pl.setColor(DPalette::Light, QColor("#25B7FF"));
        pl.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0);
        pl.setColor(DPalette::Shadow, sbcolor);
//        d->importButton->setPalette(pl);

//        d->addMusicButton->setPalette(pl);

        QPalette pa = m_text->palette();
        pa.setColor(QPalette::WindowText, "#777777");
//        d->text->setPalette(pa);
        m_text->setForegroundRole(QPalette::WindowText);
//        d->text->setForegroundRole(DPalette::TextTips);
        m_logo->setPixmap(QIcon::fromTheme("import_music_light").pixmap(QSize(128, 128)));
    } else {
        auto pl = m_importPathButton->palette();
        pl.setColor(DPalette::Dark, QColor("#0056C1"));
        pl.setColor(DPalette::Light, QColor("#004C9C"));
        pl.setColor(DPalette::ButtonText, QColor("#B8D3FF"));
        QColor sbcolor("#0091FF");
        sbcolor.setAlphaF(0.1);
        pl.setColor(DPalette::Shadow, sbcolor);
//        d->importButton->setPalette(pl);

//        d->addMusicButton->setPalette(pl);

        QPalette pa = m_text->palette();
        pa.setColor(QPalette::WindowText, "#798190");
//        d->text->setPalette(pa);
        m_text->setForegroundRole(QPalette::WindowText);
//        d->text->setForegroundRole(DPalette::TextTips);
        m_logo->setPixmap(QIcon::fromTheme("import_music_dark").pixmap(QSize(128, 128)));
    }

}

void ImportWidget::slotImportedPercent(int percent)
{
    if (m_waterProgress->isVisible()) {
        m_waterProgress->setValue(percent);
    }
}

void ImportWidget::showWaitHint()
{
    m_logo->setVisible(false);
    m_waterProgress->setVisible(true);
    m_waterProgress->start();
    m_importPathButton->setDisabled(true);
    m_importPathButton->hide();
    m_addMusicButton->hide();
    m_text->setText(tr("Loading music, please wait..."));
}
