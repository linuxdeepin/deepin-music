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

#pragma once

#include <QHBoxLayout>

#include <DWidget>
#include <DLabel>
#include <DBlurEffectWidget>
#include <QImage>

#include <searchmeta.h>

DWIDGET_USE_NAMESPACE

class Cover;
class LyricLabel;
class MusicImageButton;
class MusicLyricWidget : public DWidget
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)

public:
    explicit MusicLyricWidget(QWidget *parent = Q_NULLPTR);
    ~MusicLyricWidget() override;

    void updateUI();
    // 控件显示动画
    void showAnimation(const QSize &size);
    // 控件关闭动画
    void closeAnimation(const QSize &size);
    QString defaultCover() const;
    void checkHiddenSearch(QPoint mousePos);


signals:
    void toggleLyricView();
    void requestContextSearch(const QString &context);

public slots:
    void onProgressChanged(qint64 value, qint64 length);
    void onMusicPlayed();
    void onCoverChanged();

    void onContextSearchFinished(const QString &context,
                                 const QList<DMusic::SearchMeta> &metalist);

    void setDefaultCover(QString defaultCover);

    void onsearchBt();
    void slotonsearchresult(QString path);

    void slotTheme(int type);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QHBoxLayout *m_leftLayout;
    Cover               *m_cover              = nullptr;
    LyricLabel          *m_lyricview            = nullptr;
    DLabel              *m_nolyric              = nullptr;
    MusicImageButton    *m_serachbt = nullptr;
    QString              m_defaultCover = ":/common/image/cover_max.svg";
    bool                 m_serachflag = false;
    QImage               m_backgroundimage;
    DBlurEffectWidget   *m_backgroundW = nullptr;
    QImage               m_defaultImage = QImage(":/common/image/cover_max.svg");
};
