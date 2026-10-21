// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSIC_LOGGER_H
#define MUSIC_LOGGER_H

#include <QObject>
#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

/**
 * @brief 应用自管日志过滤规则。
 *
 * 优先级：QT_LOGGING_RULES 环境变量 < DConfig org.deepin.music 的 log_rules。
 * DConfig log_rules 变化时（调试模式通过 deepin-debug-config 调整）动态生效。
 * 必须在 QApplication/DApplication 实例化之前创建，且构造函数中
 * qunsetenv("QT_LOGGING_RULES") 之前不得有任何日志打印。
 */
class MLogger : public QObject
{
    Q_OBJECT
public:
    explicit MLogger(QObject *parent = nullptr);
    ~MLogger();

    inline QString rules() const { return m_rules; }
    void setRules(const QString &rules);

private:
    void appendRules(const QString &rules);
    void applyDConfigValue(const QByteArray &value);

private:
    QString m_rules;
    QString m_envRules;
    Dtk::Core::DConfig *m_config;
};
#endif  // MUSIC_LOGGER_H
