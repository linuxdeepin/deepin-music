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
#include "DMainWindow"

class Presenter;
class MainFramePrivate;
class MainFrame : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QString viewname READ viewname WRITE setViewname NOTIFY viewnameChanged)
public:
    explicit MainFrame(QWidget *parent = 0);
    ~MainFrame();

    void initUI(bool showLoading);
    void postInitUI();
    void binding(Presenter *presenter);

    void focusMusicList();
    QString coverBackground() const;
    QString viewname() const;

signals:
    void viewnameChanged(QString viewname);
    void addPlaylist(bool);
    void importSelectFiles(const QStringList &filelist);
    void triggerShortcutAction(const QString &optKey);

public slots:
    void setViewname(QString viewname);
    void updateUI();
    void setCoverBackground(QString coverBackground);
    void onSelectImportDirectory();
    void onSelectImportFiles();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QScopedPointer<MainFramePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MainFrame)
};

extern const QString s_PropertyViewname;
extern const QString s_PropertyViewnameLyric;
