// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DBUS_ADAPTOR
#define DBUS_ADAPTOR

#include <QtDBus>

#include "presenter.h"

class ApplicationAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.speech")

public:
    explicit ApplicationAdaptor(Presenter *pPresenter);

public Q_SLOTS:

    void OpenUris(const QStringList &listFiles);

private:
    void initMember();

private:
    Presenter *m_pPresenter;
};


#endif
