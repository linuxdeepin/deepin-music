// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicvoiceservice.h"
#include "voiceplugin.h"

MusicVoiceService::MusicVoiceService()
{
    m_voice = new VoicePlugin();
}

QString MusicVoiceService::serviceName()
{
    return "musicX";
}
int MusicVoiceService::servicePriority()
{
    return 0;
}
bool MusicVoiceService::canHandle(const QString &s)
{
    Q_UNUSED(s);
    return true;
}
IRet MusicVoiceService::service(const QString &semantic)
{
    m_voice->process(semantic);
    return ERR_SUCCESS;
}
Reply &MusicVoiceService::getServiceReply()
{
    m_reply.setReplyType(Reply::RT_STRING_TTS | Reply::RT_STRING_DISPLAY);
    m_reply.ttsMessage(m_voice->m_ttsMessage);
    return m_reply;
}
