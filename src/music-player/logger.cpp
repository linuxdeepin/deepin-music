// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logger.h"

#include <DConfig>
#include <QLoggingCategory>

DCORE_USE_NAMESPACE

MLogger::MLogger(QObject *parent)
    : QObject(parent), m_rules(""), m_config(nullptr)
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    // qunsetenv 之前一定不要有任何日志打印，否则取消环境变量设置不会生效
    qunsetenv("QT_LOGGING_RULES");

    // DTK 6.7.50+ 的 DLogManager 会异步读取 org.deepin.dtk.preference 的 rules
    // （默认 *.debug=false;*.info=false）并覆盖应用自设的过滤规则，导致
    // 重启应用后 DConfig 中设置的日志级别失效，应用自管日志规则时需禁用之
    qputenv("DTK_DISABLED_LOGGING_RULES", "1");

    // set env
    m_rules = logRules;

    // set dconfig
    m_config = DConfig::create("org.deepin.music", "org.deepin.music");
    logRules = m_config->value("log_rules").toByteArray();
    appendRules(logRules);
    setRules(m_rules);

    // watch dconfig
    connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
        if (key == "log_rules") {
            setRules(m_config->value(key).toByteArray());
        }
    });
}

MLogger::~MLogger()
{
    if (m_config) {
        m_config->deleteLater();
    }
}

void MLogger::setRules(const QString &rules)
{
    auto tmpRules = rules;
    m_rules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(m_rules);
}

void MLogger::appendRules(const QString &rules)
{
    QString tmpRules = rules;
    tmpRules = tmpRules.replace(";", "\n");
    auto tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); i++)
        if (m_rules.contains(tmplist.at(i))) {
            tmplist.removeAt(i);
            i--;
        }
    if (tmplist.isEmpty())
        return;
    m_rules.isEmpty() ? m_rules = tmplist.join("\n")
                      : m_rules += "\n" + tmplist.join("\n");
}
