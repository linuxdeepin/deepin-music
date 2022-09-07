// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H

#include "service.h"
class VoicePlugin;
class MusicVoiceService : public IService
{
public:
    MusicVoiceService();
    QString serviceName() override;
    int servicePriority() override;
    bool canHandle(const QString &s) override;
    IRet service(const QString &semantic) override;
    Reply &getServiceReply() override;

private:
    Reply                   m_reply;
    bool                    m_processResult = false;
public:
    VoicePlugin            *m_voice = nullptr;
};

#endif // WEATHERSERVICE_H
