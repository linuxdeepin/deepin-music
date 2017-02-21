#include "settings.h"

#include <QFile>
#include <qsettingbackend.h>
#include "util/global.h"

#include <QDebug>

Settings::Settings(QObject *parent) :
    Dtk::Settings(parent)
{

}

QPointer<Dtk::Settings> Settings::appSettings()
{
//    qDebug() << "new app settings";
    auto settings = Dtk::Settings::fromJsonFile(":/data/deepin-music-settings.json");
    auto configFilepath = Global::configPath() + "/config.ini";
    auto backend = new Dtk::QSettingBackend(configFilepath);
    settings->setBackend(backend);
    return settings.data();
}
