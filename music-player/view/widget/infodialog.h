/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <ddialog.h>

#include <mediameta.h>

DWIDGET_USE_NAMESPACE

class QFrame;
class QLabel;
class MediaMeta;
class InfoDialog : public DAbstractDialog
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)

    QFrame *m_infogridFrame = nullptr;
    QList<QLabel *> m_valueList;
    QLabel *m_cover = nullptr;
public:
    explicit InfoDialog(const MetaPtr meta, QWidget *parent = 0);

    void initUI(const MetaPtr meta);
    void updateLabelSize();

    QString defaultCover() const;
public slots:
    void setDefaultCover(QString defaultCover);
    void setCoverImage(const QPixmap &coverPixmap);
};

