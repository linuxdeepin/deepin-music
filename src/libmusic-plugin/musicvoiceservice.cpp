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
