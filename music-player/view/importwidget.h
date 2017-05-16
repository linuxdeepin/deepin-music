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
#include <QFrame>

class ImportWidgetPrivate;
class ImportWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ImportWidget(QWidget *parent = 0);
    ~ImportWidget();

signals:
    void scanMusicDirectory();
    void importFiles();
    void importSelectFiles(const QStringList &urllist);

public slots:
    void showWaitHint();
    void showImportHint();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<ImportWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ImportWidget)
};

