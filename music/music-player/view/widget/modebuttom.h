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

#ifndef MODEBUTTOM_H
#define MODEBUTTOM_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

class ModeButton : public DPushButton
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    explicit ModeButton(QWidget *parent = Q_NULLPTR);
    ~ModeButton() {};
    void setModeIcons(const QStringList &modeIcons, const QStringList &pressIcons = QStringList());
    int mode() const;
    void setTransparent(bool flag);
signals:
    void modeChanged(int mode);

public slots:
    void setMode(int mode);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    char        status = 0;
    int         m_mode = 0;
    bool        transparent  = true;
    QStringList m_modeIcons = QStringList();
    QStringList m_pressIcons = QStringList();
};

#endif // MODEBUTTOM_H
