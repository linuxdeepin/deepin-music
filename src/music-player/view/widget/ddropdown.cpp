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

#include "ddropdown.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>

#include <DLabel>
#include <DMenu>
#include <DIconButton>

class DDropdownPrivate
{
public:
    DDropdownPrivate(DDropdown *parent) : q_ptr(parent) {}

    DMenu            *menu       = nullptr;
    DLabel           *text       = nullptr;
    DIconButton      *dropdown   = nullptr;
    QString           status;

    DDropdown *q_ptr;
    Q_DECLARE_PUBLIC(DDropdown)
};

DDropdown::DDropdown(QWidget *parent) : DWidget(parent), d_ptr(new DDropdownPrivate(this))
{
    Q_D(DDropdown);

    setObjectName("DDropdown");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 15, 0);
    d->menu = new DMenu;
    d->text = new DLabel(tr("undefined"));
    auto font = d->text->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);
    font.setPixelSize(11);
    d->text->setFont(font);
    d->text->setObjectName("DDropdownText");
    d->text->setForegroundRole(DPalette::TextTitle);
    d->dropdown = new DIconButton(DStyle::SP_ArrowDown, this);
    d->dropdown->setIconSize(QSize(9, 9));
    d->dropdown->setObjectName("DDropdownIcon");
    d->dropdown->setFixedSize(9, 9);
    DPalette pl = d->dropdown->palette();
    pl.setColor(DPalette::Light, QColor(Qt::transparent));
    pl.setColor(DPalette::Dark, QColor(Qt::transparent));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0);
    pl.setColor(DPalette::Shadow, sbcolor);
    d->dropdown->setPalette(pl);

    layout->addStretch();
    layout->addStretch();
    layout->addWidget(d->text, 0, Qt::AlignCenter);
    layout->addWidget(d->dropdown, 0, Qt::AlignCenter);

    connect(d->menu, &DMenu::triggered, this, [ = ](QAction * action) {
        d->text->setText(action->text());
        Q_EMIT this->triggered(action);
    });

    connect(this, &DDropdown::requestContextMenu,
    this, [ = ]() {
        auto center = this->mapToGlobal(this->rect().topLeft());
        center.setY(center.y() + this->height() + 5);
        d->menu->move(center);
        d->menu->exec();
    });

    connect(d->dropdown, &DIconButton::clicked, this, [ = ](bool checked) {
        Q_EMIT requestContextMenu();
    });
}

DDropdown::~DDropdown()
{

}

QString DDropdown::status() const
{
    Q_D(const DDropdown);
    return d->status;
}

QList<QAction *> DDropdown::actions() const
{
    Q_D(const DDropdown);
    return d->menu->actions();
}

void DDropdown::setText(const QString &text)
{
    Q_D(DDropdown);
    d->text->setText(text);
}

void DDropdown::setCurrentAction(QAction *action)
{
    Q_D(DDropdown);
    if (action) {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
        d->text->setText(action->text());
        action->setChecked(true);
    } else {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
    }
}

void DDropdown::setCurrentAction(int index)
{
    Q_D(DDropdown);
    if (!d->menu->actions().isEmpty()) {
        setCurrentAction(d->menu->actions().first());
    }
}

QAction *DDropdown::addAction(const QString &item, const QVariant &var)
{
    Q_D(DDropdown);
    auto action = d->menu->addAction(item);
    auto font = action->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    action->setData(var);
    action->setCheckable(true);
    return action;
}

void DDropdown::setStatus(QString status)
{
    Q_D(DDropdown);
    d->status = status;
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
