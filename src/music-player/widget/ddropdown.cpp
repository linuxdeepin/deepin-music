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
    menu = new DMenu;

    dropdown = new DToolButton(this);
    dropdown->setCheckable(false);
    dropdown->setIcon(QIcon::fromTheme("sort_rank_texts"));
    dropdown->setIconSize(QSize(36, 36));
    dropdown->setObjectName("DDropdownIcon");
    dropdown->setFixedSize(36, 36);
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
}

DDropdown::~DDropdown()
{

}

//QString DDropdown::getStatus() const
//{
//    return status;
//}

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
