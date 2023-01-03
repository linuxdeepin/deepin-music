// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusadpator.h"

ApplicationAdaptor::ApplicationAdaptor(Presenter *pPresenter)
    : QDBusAbstractAdaptor(pPresenter)
{
    initMember();

    m_pPresenter = pPresenter;
}

void ApplicationAdaptor::initMember()
{
    m_pPresenter = nullptr;
}

void ApplicationAdaptor::OpenUris(const QStringList &listFiles)
{
    qDebug() << __func__ << listFiles;

    m_pPresenter->importMetas(listFiles, "play", true);

}
