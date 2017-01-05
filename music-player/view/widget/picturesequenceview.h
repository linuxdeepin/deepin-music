/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include <QLabel>

class PictureSequenceViewPrivate;
class PictureSequenceView : public QLabel
{
    Q_OBJECT
public:
    explicit PictureSequenceView(QWidget *parent = 0);
    ~PictureSequenceView();

    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
signals:

public slots:
    void start();
    void stop();
    void pause();
    void setPictureSequence(const QStringList &sequence);

private:
    QScopedPointer<PictureSequenceViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PictureSequenceView)
};

