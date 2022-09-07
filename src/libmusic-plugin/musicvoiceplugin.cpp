// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
