/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
