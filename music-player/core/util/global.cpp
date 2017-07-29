#include "global.h"

#include <QCoreApplication>
#include <DStandardPaths>

DCORE_USE_NAMESPACE;

QString Global::configPath()
{
    auto userConfigPath = DStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    return userConfigPath + "/" + qApp->organizationName() + "/" + qApp->applicationName();
}

QString Global::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    return userCachePath;
}

