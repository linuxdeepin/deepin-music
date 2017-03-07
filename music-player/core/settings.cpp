#include "settings.h"
#include "option.h"

#include <QFile>
#include <QFileInfo>
#include <qsettingbackend.h>
#include "util/global.h"

#include <QDebug>

Settings::Settings(QObject *parent) :
    Dtk::Settings(parent)
{

}

QPointer<Dtk::Settings> Settings::appSettings()
{
    auto settings = Dtk::Settings::fromJsonFile(":/data/deepin-music-settings.json");
    auto configFilepath = Global::configPath() + "/config.ini";
    auto needInit = !QFileInfo::exists(configFilepath);
    auto backend = new Dtk::QSettingBackend(configFilepath);
    settings->setBackend(backend);

    // TODO: fix in dtksettings
    if (needInit) {
        settings->setOption("base.play.remember_progress", false);
        settings->setOption("base.play.remember_progress", true);
        settings->setOption("base.play.last_playlist", "");
        settings->setOption("base.play.last_playlist", "all");
    }

    return settings.data();
}
