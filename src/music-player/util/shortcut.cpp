// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut.h"
#include "util/log.h"

#include <QVariant>
#include <QProcess>
#include <QRect>

#include "presenter.h"

struct ShortcutItem {
    QString name;
    QString value;
    ShortcutItem(const QString &n, const QString &v): name(n), value(v) {}
};

struct ShortcutGroup {
    QString groupName;
    QList<ShortcutItem> groupItems;
};

class ShortcutPrivate
{
public:
    ShortcutPrivate(Shortcut *parent)
        : m_parent(parent)
    {
        qCDebug(dmMusic) << "ShortcutPrivate constructor";
    }

private:
    friend class Shortcut;
    Shortcut                  *m_parent;
};

Shortcut::Shortcut(QObject *parent)
    : QObject(parent), m_data(new ShortcutPrivate(this))
{
    qCDebug(dmMusic) << "Shortcut constructor";
}

void Shortcut::show(const int &x, const int &y)
{
    qCDebug(dmMusic) << "Shortcut show";
    Presenter *presenter = dynamic_cast<Presenter *>(parent());
    if (presenter == nullptr) {
        qCWarning(dmMusic) << "Shortcut show failed, presenter is nullptr";
        return;
    }

    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;

    group1.groupName = tr("Play");
    group2.groupName = tr("Songs");
    group3.groupName = tr("Playlists");
    group4.groupName = tr("Settings");
    auto strPlayPause = presenter->valueFromSettings("shortcuts.all.play_pause").toString();
    auto strPrevious = presenter->valueFromSettings("shortcuts.all.previous").toString();
    auto strNext = presenter->valueFromSettings("shortcuts.all.next").toString();
    auto strVolUP = presenter->valueFromSettings("shortcuts.all.volume_up").toString();
    auto strVolDown = presenter->valueFromSettings("shortcuts.all.volume_down").toString();
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

    QList<ShortcutGroup> shortcutGroups;
    shortcutGroups << group1 << group2 << group3 << group4;

    //convert to json object
    QJsonArray jsonGroups;
    for (auto scg : shortcutGroups) {
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
    QJsonObject shortcutObj;
    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QPoint pos(x, y);
    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProc = new QProcess(parent());
    //此处不会造成多进程闲置，deepin-shortcut会自动检查删除多余进程
    shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));
    qCDebug(dmMusic) << "Shortcut show end";
}
