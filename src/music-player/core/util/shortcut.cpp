// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut.h"
#include "./core/musicsettings.h"
#include <QVariant>
Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;

    group1.groupName = tr("Play");
    group2.groupName = tr("Songs");
    group3.groupName = tr("Playlists");
    group4.groupName = tr("Settings");
    auto strPlayPause = MusicSettings::value("shortcuts.all.play_pause").toString();
    auto strPrevious = MusicSettings::value("shortcuts.all.previous").toString();
    auto strNext = MusicSettings::value("shortcuts.all.next").toString();
    auto strVolUP = MusicSettings::value("shortcuts.all.volume_up").toString();
    auto strVolDown = MusicSettings::value("shortcuts.all.volume_down").toString();
    group1.groupItems <<
                      ShortcutItem(tr("Play/Pause"),    strPlayPause.toUpper().compare("RETURN") == 0 ? "Enter" : strPlayPause) <<
                      ShortcutItem(tr("Previous"),      strPrevious.toUpper().compare("RETURN") == 0 ? "Enter" : strPrevious) <<
                      ShortcutItem(tr("Next"),          strNext.toUpper().compare("RETURN") == 0 ? "Enter" : strNext) <<
                      ShortcutItem(tr("Volume Up"),     strVolUP.toUpper().compare("RETURN") == 0 ? "Enter" : strVolUP) <<
                      ShortcutItem(tr("Volume Down"),   strVolDown.toUpper().compare("RETURN") == 0 ? "Enter" : strVolDown) <<
                      ShortcutItem(tr("Mute"),   "M");

    group2.groupItems <<
                      ShortcutItem(tr("Favorite"), ".") <<
                      ShortcutItem(tr("Unfavorite"),  ".") <<
                      ShortcutItem(tr("Song info"),   "Ctrl+I");

    group3.groupItems <<
                      ShortcutItem(tr("New playlist"), "Ctrl+Shift+N") <<
                      ShortcutItem(tr("Add music"), "Ctrl+O") <<
                      ShortcutItem(tr("Rename playlist"), "F2") <<
                      ShortcutItem(tr("Remove from playlist"), "Delete");

    group4.groupItems <<
                      ShortcutItem(tr("Help"),  "F1") <<
                      ShortcutItem(tr("Display shortcuts"), "Ctrl+Shift+?");

    m_shortcutGroups << group1 << group2 << group3 << group4;

    //convert to json object
    QJsonArray jsonGroups;
    for (auto scg : m_shortcutGroups) {
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName", scg.groupName);
        QJsonArray jsonGroupItems;
        for (auto sci : scg.groupItems) {
            QJsonObject jsonItem;
            jsonItem.insert("name", sci.name);
            jsonItem.insert("value", sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems", jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut", jsonGroups);
}
QString Shortcut::toStr()
{
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
