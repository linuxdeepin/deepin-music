#include "global.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcessEnvironment>

QString Global::configPath()
{
#ifdef SNAP_APP
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString SNAP_USER_COMMON = env.value("SNAP_USER_COMMON");
    auto userConfigPath = SNAP_USER_COMMON;
#else
    auto userConfigPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
#endif

    return userConfigPath + "/" + qApp->organizationName() + "/" + qApp->applicationName();
}

QString Global::cacheDir()
{

#if SNAP_APP
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString SNAP_USER_COMMON = env.value("SNAP_USER_COMMON");
    auto userCachePath = SNAP_USER_COMMON;
#else
    auto userCachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
#endif

    return userCachePath;
}

