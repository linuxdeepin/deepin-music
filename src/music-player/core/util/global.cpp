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

#include "global.h"

#include <QCoreApplication>
#include <DStandardPaths>

#include <QDir>
#include <QFile>
#include <QFileInfo>

#define RECURSION_LIMIT 5

DCORE_USE_NAMESPACE;
QString appName;

static int recursion = 0;
static bool firstTime = true;
static QString imagePushFilePath = "/tmp/deepin-music/cover.jpg";

QString Global::configPath()
{
    auto userConfigPath = DStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).value(0);
    return userConfigPath;
}

QString Global::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    return userCachePath;
}

void Global::setAppName(QString name)
{
    appName = name;
}

QString Global::getAppName()
{
    return appName;
}

QString Global::imagePushed(QFile &src)
{
    QFileInfo info(imagePushFilePath);
    if ((!info.exists() | !info.isWritable()) && firstTime == false && recursion<RECURSION_LIMIT) {
        QString path = imagePushFilePath.remove(info.fileName());
        path.chop(1);
        path += "_/cover.jpg";
        imagePushFilePath = path;
        QDir().mkdir(path);
        recursion++;
        return imagePushed(src);
    } else {
        if (firstTime) {
            firstTime = false;
            //So create the dir first.
            if (!QDir().mkdir("/tmp/deepin-music")) {
                return imagePushed(src);
            }
        }
        QFile f(imagePushFilePath);
        if (f.open(QIODevice::OpenModeFlag::Truncate | QIODevice::OpenModeFlag::WriteOnly)) {
            //SRC file checks, else abort OP
            if (!src.isOpen()) {
                if (!src.open(QIODevice::OpenModeFlag::ReadOnly)) {
                    return "";
                }
            }
            if (src.isReadable()) {
                f.write(src.readAll(), src.size());
                f.flush();
                f.close();
                recursion = 0;
                return imagePushFilePath;
            }
            return "";
        } else if (recursion<RECURSION_LIMIT) {//We can remove it as at a time it'll for sure find a good path to use
            //Maybe an error we haven't caught.
            QString path = imagePushFilePath.remove(info.fileName());
            path.chop(1);
            path += "_/cover.jpg";
            imagePushFilePath = path;
            QDir().mkdir(path);
            recursion++;
            return imagePushed(src);
        } else {
            recursion = 0;
            return "";
        }
    }
}
