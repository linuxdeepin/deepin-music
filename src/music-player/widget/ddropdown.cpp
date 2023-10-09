// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddropdown.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>

#include <DLabel>
#include <DMenu>
#include <DIconButton>
#include <DFontSizeManager>

DDropdown::DDropdown(QWidget *parent) : DWidget(parent)
{
    setObjectName("DDropdown");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    menu = new DMenu(this);//添加父类，减少内存泄露的风险

    dropdown = new DToolButton(this);
    dropdown->setCheckable(false);
    dropdown->setIcon(QIcon::fromTheme("sort_rank_texts"));
    dropdown->setIconSize(QSize(36, 36));
    dropdown->setObjectName("DDropdownIcon");
#ifdef DTKWIDGET_CLASS_DSizeMode
    slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
#else
    dropdown->setFixedSize(36, 36);
#endif
    dropdown->setWindowModality(Qt::WindowModal);
    DPalette pl = dropdown->palette();
    pl.setColor(DPalette::Light, QColor(Qt::transparent));
    pl.setColor(DPalette::Dark, QColor(Qt::transparent));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0);
    pl.setColor(DPalette::Shadow, sbcolor);
    dropdown->setPalette(pl);

    layout->addStretch();
    layout->addStretch();
    layout->addWidget(dropdown, 0, Qt::AlignCenter);

    connect(menu, &DMenu::triggered, this, [ = ](QAction * action) {
        Q_EMIT this->triggered(action);
    });

    connect(this, &DDropdown::requestContextMenu,
    this, [ = ]() {
        auto center = this->mapToGlobal(this->rect().topLeft());
        center.setY(center.y() + this->height() + 5);
        menu->move(center);
        menu->exec();
    });

    connect(dropdown, &DIconButton::clicked, this, [ = ](bool checked) {
        Q_UNUSED(checked)
        Q_EMIT requestContextMenu();
    });
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &DDropdown::slotSizeModeChanged);
#endif
}

DDropdown::~DDropdown()
{

}

QList<QAction *> DDropdown::actions() const
{
    return menu->actions();
}

void DDropdown::setCurrentAction(QAction *action)
{
    if (action) {
        for (auto action : menu->actions()) {
            action->setChecked(false);
        }
        action->setChecked(true);
    } else {
        for (auto action : menu->actions()) {
            action->setChecked(false);
        }
    }
}

void DDropdown::setCurrentAction(int index)
{
    Q_UNUSED(index)
    if (!menu->actions().isEmpty()) {
        setCurrentAction(menu->actions().first());
    }
}

QAction *DDropdown::addAction(const QString &item, const QVariant &var)
{
    auto action = menu->addAction(item);
    auto font = action->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    action->setData(var);
    action->setCheckable(true);
    return action;
}

void DDropdown::setStatus(const QString &status)
{
    this->status = status;
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void DDropdown::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        dropdown->setFixedSize(24, 24);
    } else {
        dropdown->setFixedSize(36, 36);
    }
}
#endif

void DDropdown::enterEvent(QEvent *event)
{
    setStatus("hover");
    DWidget::enterEvent(event);

    update();
}

void DDropdown::leaveEvent(QEvent *event)
{
    setStatus("");
    DWidget::leaveEvent(event);

    update();
}

void DDropdown::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT requestContextMenu();
    }
    DWidget::mouseReleaseEvent(event);
}
