#include "global.h"

#include <QCoreApplication>
#include <QStandardPaths>

QString Global::configPath()
{
    auto userConfigPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    return userConfigPath + "/" + qApp->organizationName() + "/" + qApp->applicationName();
}

QString Global::cacheDir()
{
    auto userCachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    return userCachePath;
}
