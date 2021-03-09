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

#include "infodialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileInfo>
#include <QScrollArea>

#include <DGuiApplicationHelper>
#include <DApplication>
#include "dplatformwindowhandle.h"
#include "dblureffectwidget.h"
#include <DFrame>
#include <DFloatingWidget>
#include <DHiDPIHelper>
#include <DWindowCloseButton>
#include <DFontSizeManager>
#include <DArrowLineDrawer>

#include <dwindowclosebutton.h>

#include "cover.h"
#include "global.h"

DWIDGET_USE_NAMESPACE

static const int CoverSize = 142;

QImage cropRect(const QImage &image, QSize sz)
{
    // 添加非空判断，减少警告日志输出与不必要的性能损耗
    QImage newImage;
    if (!image.isNull()) {
        newImage = image.scaled(sz, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
    auto center = QRectF(newImage.rect()).center();
    auto topLeft = QPointF(center.x() - sz.width() / 2.0,
                           center.y() - sz.height() / 2.0);
    if (topLeft.x() < 0) {
        topLeft.setX(0);
    }
    if (topLeft.y() < 0) {
        topLeft.setY(0);
    }

    QRect crop(topLeft.toPoint(), sz);
    return newImage.copy(crop);
}

void InfoDialog::initUI()
{
    setObjectName("infoDialog");
    setFixedSize(320, 500);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(10, 50, 10, 10);

    closeBt = new DWindowCloseButton(this);
    closeBt->setFocusPolicy(Qt::NoFocus);
    closeBt->setFixedSize(50, 50);
    closeBt->setIconSize(QSize(50, 50));
    closeBt->move(width() - 50, 0);

    cover = new Cover(this);
    cover->setContentsMargins(0, 0, 0, 0);
    cover->setObjectName("InfoCover");
    cover->setFixedSize(CoverSize, CoverSize);
    cover->move(89, 60);
    title = new DLabel(this);
    title->setObjectName("InfoTitle");
    title->setFixedWidth(290);
    title->setAlignment(Qt::AlignCenter);
    title->setWordWrap(true);
    title->setForegroundRole(DPalette::BrightText);
    title->move(10, 212);

    infoGridFrame = new DFrame(this);
    infoGridFrame->setFocusPolicy(Qt::NoFocus);
    infoGridFrame->setLineWidth(0);
    infoGridFrame->setFrameRounded(true);
    infoGridFrame->setMaximumWidth(300);
    infoGridFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DPalette pl = infoGridFrame->palette();
    QColor windowColor("#FFFFFF");
    windowColor.setAlphaF(0.7);
    pl.setColor(DPalette::Window, windowColor);
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0.05);
    pl.setColor(DPalette::Shadow, sbcolor);
    infoGridFrame->setPalette(pl);

    layout->addSpacing(10);
    layout->addWidget(cover, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(title, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(infoGridFrame);

    basicinfo = new DLabel("   " + InfoDialog::tr("Basic info"), this);

    basicinfo->setForegroundRole(DPalette::Text);
    basicinfo->setFixedWidth(300);

    auto infoLayout = new QVBoxLayout(infoGridFrame);
    infoLayout->setSpacing(0);
    infoLayout->setMargin(0);

    auto infogridLayout = new QGridLayout(infoGridFrame);
    infogridLayout->setMargin(10);
    infogridLayout->setHorizontalSpacing(10);
    infogridLayout->setVerticalSpacing(5);
    infogridLayout->setColumnStretch(0, 60);
    infogridLayout->setColumnStretch(1, 260);

    QStringList infoKeys;
    infoKeys << InfoDialog::tr("Title:") << InfoDialog::tr("Artist:")
             << InfoDialog::tr("Album:") << InfoDialog::tr("Type:")
             << InfoDialog::tr("Size:") << InfoDialog::tr("Duration:")
             << InfoDialog::tr("Path:");

    for (int i = 0; i < infoKeys.length(); ++i) {
        auto infoKey = new DLabel(infoKeys.value(i));
//        auto infoFont = infoKey->font();
//        infoFont.setPointSize(8);
//        infoKey->setFont(infoFont);
        infoKey->setObjectName("InfoKey");
//        infoKey->setMinimumHeight(18);
        infoKey->setForegroundRole(DPalette::WindowText);
        infoKey->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(infoKey, DFontSizeManager::T8);
        keyList << infoKey;

        auto infoValue = new DLabel(this);
//        infoValue->setFont(infoFont);
        infoValue->setWordWrap(true);
        infoValue->setObjectName("InfoValue");
//        infoValue->setMinimumHeight(28);
        infoValue->setMinimumWidth(200);
        infoValue->setMaximumWidth(260);
        infoValue->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        infoValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        infoValue->setForegroundRole(DPalette::WindowText);
        DFontSizeManager::instance()->bind(infoValue, DFontSizeManager::T8);
        valueList << infoValue;

        infogridLayout->addWidget(infoKey);
        infogridLayout->addWidget(infoValue);
    }
    infoLayout->addWidget(basicinfo);
    infoLayout->addLayout(infogridLayout);

    connect(closeBt, &DIconButton::clicked, this, &DAbstractDialog::hide);
    connect(qApp, &QGuiApplication::fontChanged, this, [ = ](const QFont & font) {
        QFontMetrics fm(font);
        for (int i = 0; i < keyList.size(); i++) {

            int w = keyList.at(0)->width();
            int value_w = 300 - w;
            valueList.at(i)->setFixedWidth(value_w - 20);
        }
        if (meta.size > 1.0) {
            updateInfo(meta);
        }
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &InfoDialog::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());

    updateInfo(meta);
}


void InfoDialog::updateLabelSize()
{
    title->adjustSize();
    auto h = 0;
    for (auto label : valueList) {
        h += label->size().height() + 6 + 6;
    }
    infoGridFrame->setFixedHeight(h);
    infoGridFrame->adjustSize();
    adjustSize();
}

InfoDialog::InfoDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    initUI();
}

InfoDialog::~InfoDialog()
{

}

void InfoDialog::resizeEvent(QResizeEvent *event)
{
    //Q_D(InfoDialog);
    Dtk::Widget::DAbstractDialog::resizeEvent(event);
}

void InfoDialog::updateInfo(const MediaMeta meta)
{
    QString artist =  meta.singer.isEmpty() ? tr("Unknown artist") :  meta.singer;
    QString album =  meta.album.isEmpty() ? tr("Unknown album") :  meta.album;
    QStringList infoValues;
    infoValues <<  meta.title << artist << album
               <<  meta.filetype << DMusic::sizeString(meta.size) << DMusic::lengthString(meta.length)
               <<  meta.localPath;

    for (int i = 0; i < valueList.length(); ++i) {
        if (i != valueList.length() - 1) {
            QString str = geteElidedText(valueList.value(i)->font(), infoValues.value(i), valueList.value(i)->width());
            valueList.value(i)->setText(str);
            QFontMetrics fontWidth(valueList.value(i)->font());
            QRect rec = fontWidth.boundingRect(valueList.value(i)->text());
            valueList.value(i)->setFixedHeight(rec.height());
        } else {
            QFontMetrics fontWidth(valueList.value(i)->font());
            int width = fontWidth.width(infoValues.value(i));  //计算字符串宽度
            if (width >= valueList.value(i)->width()) { //当字符串宽度大于最大宽度时进行转换
                //两行
                DoubleElements = true;
                QString str = geteElidedText(valueList.value(i)->font(), infoValues.value(i), valueList.value(i)->width() * 3 / 2);
                valueList.value(i)->setText(str);
                QRect rec = fontWidth.boundingRect(valueList.value(i)->text());
                valueList.value(i)->setFixedHeight(2 * rec.height());

            } else {
                DoubleElements = false;
                //QString str = geteElidedText(d->valueList.value(i)->font(), infoValues.value(i), d->valueList.value(i)->width() / 2);
                valueList.value(i)->setText(infoValues.value(i));
                QRect rec = fontWidth.boundingRect(valueList.value(i)->text());
                valueList.value(i)->setFixedHeight(rec.height());
            }
        }

        QFileInfo fileInfo(meta.localPath);
        QString titleStr(meta.title + "-" +  meta.singer + "." + fileInfo.suffix());
        titleStr = geteElidedText(title->font(), titleStr, title->width());
        title->setText(titleStr);

        auto pixmapSize = static_cast<int>(CoverSize * cover->devicePixelRatioF());

        MediaMeta cmeta = meta;

        QPixmap coverPixmap;
        QImage coverimg;
        QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
        if (coverInfo.exists()) {
            coverimg = QImage(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
            coverPixmap = QPixmap::fromImage(cropRect(coverimg, QSize(pixmapSize, pixmapSize)));
        } else {
            coverPixmap = QIcon::fromTheme("cover_max").pixmap(QSize(pixmapSize, pixmapSize));
        }
        coverPixmap.setDevicePixelRatio(cover->devicePixelRatioF());
        cover->setCoverPixmap(coverPixmap);
        cover->update();
        updateLabelSize();

        title->setFocus();
    }

    int h = 0;//one Label Height
    for (int i = 0; i < valueList.size(); i++) {
        h = valueList.value(i)->height();
        if (h != 0)
            break;
    }
    int title_height = title->height();
    int grideframe_height = infoGridFrame->height();
    int total = title_height + grideframe_height + 192 + 70;
    if (DoubleElements) {
        setFixedHeight(total + h);
    } else {
        setFixedHeight(total + 22);
    }
}

void InfoDialog::setThemeType(int type)
{
    if (type == 1) {
        DPalette framePl = this->palette();
        QColor frameColor("#F7F7F7");
        frameColor.setAlphaF(0.8);
        framePl.setColor(DPalette::Window, frameColor);
        this->setBackgroundRole(DPalette::Window);

        DPalette pl = infoGridFrame->palette();
        QColor windowColor("#FFFFFF");
        windowColor.setAlphaF(0.7);
        pl.setColor(DPalette::Window, windowColor);

        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        pl.setColor(DPalette::Shadow, sbcolor);
        infoGridFrame->setPalette(pl);
        infoGridFrame->setBackgroundRole(DPalette::Window);
    } else {
        DPalette framePl = this->palette();
        QColor frameColor("#191919");
        frameColor.setAlphaF(0.8);
        framePl.setColor(DPalette::Window, frameColor);
        this->setBackgroundRole(DPalette::Window);

        DPalette pl = infoGridFrame->palette();
        QColor windowColor("#282828");
        //windowColor.setAlphaF(0.05);
        pl.setColor(DPalette::Window, windowColor);
        QColor sbcolor("#FFFFFF");
        sbcolor.setAlphaF(0.1);
        pl.setColor(DPalette::Shadow, sbcolor);
        infoGridFrame->setPalette(pl);
        infoGridFrame->setBackgroundRole(DPalette::Window);
    }
}

QString InfoDialog::geteElidedText(QFont font, QString str, int MaxWidth)
{
    QFontMetrics fontWidth(font);
    int width = fontWidth.width(str) + 10;  //计算字符串宽度,+10提前进入省略，避免右边遮挡
    if (width >= MaxWidth) { //当字符串宽度大于最大宽度时进行转换
        str = fontWidth.elidedText(str, Qt::ElideMiddle, MaxWidth); //中间显示省略号
    }
    return str;
}
