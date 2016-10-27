/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class MusicMeta;
class InfoDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit InfoDialog(const MusicMeta& info, QWidget *parent = 0);

signals:

public slots:
};

#endif // INFODIALOG_H
