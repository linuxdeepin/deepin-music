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

#include "widget/thinwindow.h"

class Presenter;
class MainFramePrivate;
class MainFrame : public ThinWindow
{
    Q_OBJECT
public:
    explicit MainFrame(QWidget *parent = 0);
    ~MainFrame();

    void binding(Presenter *presenter);

    void focusMusicList();
    QString coverBackground() const;

signals:
    void addPlaylist(bool );
    void importSelectFiles(const QStringList &filelist);

public slots:
    void updateUI();
    void setCoverBackground(QString coverBackground);
    void onSelectImportDirectory();
    void onSelectImportFiles();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<MainFramePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MainFrame)
};

extern const QString s_PropertyViewname;
extern const QString s_PropertyViewnameLyric;
