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

DWIDGET_USE_NAMESPACE

class MusicMeta;
class InfoDialog : public DAbstractDialog
{
    Q_OBJECT
    Q_PROPERTY(QString defaultCover READ defaultCover WRITE setDefaultCover)

    QString m_cover;
public:
    explicit InfoDialog(const MusicMeta &info, const QPixmap &coverPixmap, QWidget *parent = 0);

    QString defaultCover() const;
public slots:
    void setDefaultCover(QString defaultCover);
};

