// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <DFrame>
#include <DPushButton>
#include <QLabel>
#include <DWaterProgress>

DWIDGET_USE_NAMESPACE

class ImportWidgetPrivate;
class ImportWidget : public DFrame
{
    Q_OBJECT
public:
    explicit ImportWidget(QWidget *parent = Q_NULLPTR);
    ~ImportWidget() override;

    // 向下移动隐藏
    void closeAnimationToDown(const QSize &size);
    // 向左移动显示
    void showAnimationToLeft(const QSize &size);

    const QString getLastImportPath() const;
    void addMusic(QString listHash);
signals:
    void scanMusicDirectory();
    void selectImportDirectory();
    void importSelectFiles(const QStringList &urllist);
    void signalImportWidgetShowAndClose();

public slots:
    void showWaitHint();
    void showImportHint();
    void slotLinkActivated(const QString &link);
    void slotAddMusicButtonClicked();
    void slotImportPathButtonClicked();
    //void slotImportFormDbus(const QUrl &url); //右键菜单打开音乐
    void setThemeType(int type);
    // 已导入百分比
    void slotImportedPercent(int percent);
    // 将文件拖动到任务栏上的音乐图标导入
    void slotFileImportProcessing(const QStringList &itemMetas);
protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel                  *m_text = nullptr;
    DPushButton             *m_importPathButton = nullptr;
    DPushButton             *m_addMusicButton = nullptr;
    QLabel                  *m_logo = nullptr;
    DWaterProgress          *m_waterProgress = nullptr;
};

