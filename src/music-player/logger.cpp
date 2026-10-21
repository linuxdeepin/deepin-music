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
    m_envRules = logRules;

    // set dconfig：schema 未安装或配置文件缺失时 create 可能返回 nullptr，
    // 此时降级为仅使用环境变量规则，避免解引用空指针崩溃
    m_config = DConfig::create("org.deepin.music", "org.deepin.music");
    if (m_config) {
        applyDConfigValue(m_config->value("log_rules").toByteArray());
        // watch dconfig
        connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
            if (key == "log_rules") {
                applyDConfigValue(m_config->value(key).toByteArray());
            }
        });
    } else {
        setRules(m_envRules);
    }
}

MLogger::~MLogger()
{
    // MLogger 通常为 main 中的栈对象，应用退出时事件循环已停止，
    // deleteLater 投递的延迟删除事件不会被处理，因此直接 delete
    delete m_config;
    m_config = nullptr;
}

void MLogger::setRules(const QString &rules)
{
    auto tmpRules = rules;
    m_rules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(m_rules);
}

void MLogger::applyDConfigValue(const QByteArray &value)
{
    // 以环境变量规则为基底，DConfig 规则追加其后（Qt 规则后生效者优先），
    // 保证 valueChanged 后环境变量规则不丢失且 DConfig 优先级更高。
    // 注意：appendRules 只更新 m_rules 不应用，最后必须 setRules 应用生效
    setRules(m_envRules);
    appendRules(value);
    setRules(m_rules);
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
