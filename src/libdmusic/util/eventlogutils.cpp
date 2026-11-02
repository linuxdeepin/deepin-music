// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLibrary>
#include <QJsonDocument>

#include "eventlogutils.h"
#include "log.h"

EventLogUtils *EventLogUtils::m_instance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    if (m_instance == nullptr) {
        m_instance = new EventLogUtils;
    }
    return *m_instance;
}

EventLogUtils::EventLogUtils()
{
    QLibrary library("libdeepin-event-log.so");

    init = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr) {
        qCDebug(dmMusic) << "Initialize function not resolved. Event logging will be disabled.";
        return;
    }
    init("deepin-music", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    if (writeEventLog == nullptr) {
        qCDebug(dmMusic) << "writeEventLog function not resolved. Skipping log write.";
        return;
    }

    std::string logString = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(logString);
}
