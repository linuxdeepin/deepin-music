// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTLOGUTILS_H
#define EVENTLOGUTILS_H

#include <QJsonObject>
#include <string>

class EventLogUtils
{
public:
    static EventLogUtils &get();
    void writeLogs(QJsonObject &data);

private:
    bool (*init)(const std::string &packagename, bool enable_sig) = nullptr;
    void (*writeEventLog)(const std::string &eventdata) = nullptr;

    static EventLogUtils *m_instance;

    EventLogUtils();
};

#endif // EVENTLOGUTILS_H
