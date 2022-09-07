// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_SEMANTIC_PLUGIN_H
#define SYSTEM_SEMANTIC_PLUGIN_H
#include <QObject>
#include "service.h"


#if defined(SYSTEM_SEMANTIC_LIBRARY)
#  define SYSTEMSEMANTICSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SYSTEMSEMANTICSHARED_EXPORT Q_DECL_IMPORT
#endif
#define AIAPluginProtocolVersion 1
class SYSTEMSEMANTICSHARED_EXPORT IServicePlugin
{

    typedef   std::function<void(IServicePlugin *, Reply)>  replyCallback;

public:
    virtual ~IServicePlugin() {}

    virtual int init() = 0;         /* 加载配置，初始化整个插件或服务 */
    virtual void uninit() = 0;      /* 插件逆向初始化 */

    virtual QStringList getSupportService() = 0;
    virtual IService *createService(const QString &service) = 0; /* 获取一个服务，为多线程考量新建服务 */
    virtual void releaseService(IService *service) = 0; /* 释放一个服务 */
    virtual bool needRunInDifferentThread() { return true;} /*当前线程是否需要在单独的线程中运行*/
    virtual bool isAutoDetectSession() { return true;} /*是否自动检测多轮对话*/
    virtual int getPluginProtocolVersion() { return AIAPluginProtocolVersion;}
    virtual void setMessageHandle(replyCallback callback)
    {
        m_messageHandle = callback;
    }
protected:
    replyCallback m_messageHandle;
};

#define PluginInterface_iid "com.deepin.iflytek.aiassistant.3"
Q_DECLARE_INTERFACE(IServicePlugin, PluginInterface_iid)
#endif // SYSTEM_SEMANTIC_PLUGIN_H
