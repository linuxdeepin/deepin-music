// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusadpator.h"
#include "util/log.h"

ApplicationAdaptor::ApplicationAdaptor(Presenter *pPresenter)
    : QDBusAbstractAdaptor(pPresenter)
{
    qCDebug(dmMusic) << "ApplicationAdaptor constructor";
    initMember();

    m_pPresenter = pPresenter;
}

void ApplicationAdaptor::initMember()
{
    qCDebug(dmMusic) << "ApplicationAdaptor initMember";
    m_pPresenter = nullptr;
}

void ApplicationAdaptor::OpenUris(const QStringList &listFiles)
{
    qCDebug(dmMusic) << "ApplicationAdaptor OpenUris";

    m_pPresenter->importMetas(listFiles, "play", true);

}
