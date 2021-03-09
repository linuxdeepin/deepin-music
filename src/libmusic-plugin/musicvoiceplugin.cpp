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

#include "musicvoiceplugin.h"
#include "musicvoiceservice.h"
#include "voiceplugin.h"
MusicVoicePlugin::MusicVoicePlugin(QObject *parent)
{
    Q_UNUSED(parent);
}
MusicVoicePlugin::~MusicVoicePlugin()
{

}

int MusicVoicePlugin::init()
{
    return 0;
}
void MusicVoicePlugin::uninit()
{

}
QStringList MusicVoicePlugin::getSupportService()
{
    return QStringList{ "musicX" };
}
IService *MusicVoicePlugin::createService(const QString &service)
{
    if (!getSupportService().contains(service)) {
        return nullptr;
    }
    QMutexLocker lock(&serviceLock);
    MusicVoiceService *pService = new MusicVoiceService();
    connect(pService->m_voice, &VoicePlugin::signaleSendMessage, this, &MusicVoicePlugin::slotSendMessage);
    serviceSet.insert(pService);
    return pService;
}
void MusicVoicePlugin::releaseService(IService *service)
{
    QMutexLocker lock(&serviceLock);
    for (auto iter = serviceSet.begin(); iter != serviceSet.end(); iter++) {
        if (service && service == *iter) {
            serviceSet.erase(iter);
            break;
        }
    }
    if (service) {
        delete service;
    }
}

bool MusicVoicePlugin::needRunInDifferentThread()
{
    return false;
}

void MusicVoicePlugin::slotSendMessage(QString text)
{
    qDebug() << __FUNCTION__ << text;
    Reply reply;
    reply.setReplyType(Reply::RT_STRING_TTS | Reply::RT_STRING_DISPLAY);
    reply.ttsMessage(text);
    reply.displayMessage(text);
    if (this->m_messageHandle)
        this->m_messageHandle(this, reply);
}
