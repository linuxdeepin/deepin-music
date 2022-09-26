// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEATHERPLUGIN_H
#define WEATHERPLUGIN_H
#include "interface/service.h"
#include "systemsemanticplugin.h"
#include <QMutex>
class MusicVoicePlugin : public QObject, public IServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(IServicePlugin)
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
public:
    explicit MusicVoicePlugin(QObject *parent = nullptr);
    virtual ~MusicVoicePlugin() override;

    virtual int init() override;
    virtual void uninit() override;

    virtual QStringList getSupportService() override;
    virtual IService *createService(const QString &service) override;
    virtual void releaseService(IService *service) override;
    virtual bool needRunInDifferentThread()  override;
public slots:
    void slotSendMessage(QString text);
private:
    QSet<IService *> serviceSet;
    QMutex serviceLock;
};


#endif // WEATHERPLUGIN_H
