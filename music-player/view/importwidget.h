/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef IMPORTVIEW_H
#define IMPORTVIEW_H

#include <QFrame>

class ImportWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ImportWidget(QWidget *parent = 0);

signals:
    void importMusicDirectory();
    void importFiles();

public slots:
};

#endif // IMPORTVIEW_H
