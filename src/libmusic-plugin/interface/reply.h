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

#ifndef REPLY_H
#define REPLY_H

#include <QString>
#include <QVariant>
#include <QWidget>
#include <QDebug>
enum IRet {
    ERR_SUCCESS = (0),                   /* 一切正常 */
    ERR_FAIL = (-1),                     /* 常规错误 */
    ERR_PARAM = (-2),                    /* 函数入参异常 */
    ERR_CONFIG = (-3),                   /* 配置错误 */
    ERR_NOT_FOUND = (-4),                /* 未找到对应项 */

    ERR_UNSUPPORT_SERVICE = (-20),        /* 不支持的服务 */
    ERR_UNSUPPORT_INTENT = (-21),         /* 不支持的操作 */
    ERR_UNSUPPORT_SLOTS = (-22),          /* 不支持的槽位 */
    ERR_UNSUPPORT_ABILITY = (-23),        /* 不支持的能力 */
    ERR_UNSUPPORT_EXPRESS = (-24),        /* 不支持的说法 */

    ERR_CLOUD_SERVICE = (-100),          /* 云端服务内容异常 */
    ERR_CLOUD_INTENT = (-101),           /* 云端意图内容异常 */
    ERR_CLOUD_SLOTS = (-102)             /* 云端参数内容异常 */
};
class Reply
{

public:
    enum ReplyType {
        RT_NONE = 0,
        RT_STRING_DISPLAY = 1,
        RT_STRING_TTS = 2,
        RT_POP_WIDGET = 4,
        RT_INNER_WIDGET = 8
    };
public:
    Reply()
        : messageCode(ERR_FAIL),
          messageDisplay(),
          messageTTS(),
          m_widget(nullptr),
          m_replyType(RT_NONE),
          m_shouldEndSession(true)
    {
    }

    Reply(int msgCode, const QString &message, int type = RT_STRING_TTS | RT_STRING_DISPLAY)
        : messageCode(msgCode),
          messageDisplay(message),
          messageTTS(message),
          m_widget(nullptr),
          m_replyType(type),
          m_shouldEndSession(true)
    {
    }

    Reply(int msgCode, const QString &msgDisplay, const QString &msgSynthesis, int type, QWidget *parent)
        : messageCode(msgCode),
          messageDisplay(msgDisplay),
          messageTTS(msgSynthesis),
          m_widget(parent),
          m_replyType(type),
          m_shouldEndSession(true)

    {
    }

    Reply(const Reply &reply)
        : messageCode(reply.messageCode),
          messageDisplay(reply.messageDisplay),
          messageTTS(reply.messageTTS),
          m_widget(reply.m_widget),
          m_replyType(reply.m_replyType),
          m_shouldEndSession(reply.m_shouldEndSession)
    {
    }

    Reply &operator=(const Reply &reply)
    {
        this->messageCode = reply.messageCode;
        this->messageDisplay = reply.messageDisplay;
        this->messageTTS = reply.messageTTS;
        this->m_widget = reply.m_widget;
        this->m_replyType = reply.m_replyType;
        m_shouldEndSession = reply.m_shouldEndSession;
        return *this;
    }

    int code()
    {
        if (messageCode.isValid()) {
            return messageCode.toInt();
        } else {
            return ERR_SUCCESS;
        }
    }

    void code(int msgCode)
    {
        this->messageCode = msgCode;
    }

    const QString displayMessage()
    {
        if (messageDisplay.isValid()) {
            return messageDisplay.toString();
        } else {
            return "";
        }
    }

    void displayMessage(const QString &msgDisplay)
    {
        this->messageDisplay = msgDisplay;
    }

    const QString ttsMessage()
    {
        if (messageTTS.isValid()) {
            return messageTTS.toString();
        } else {
            return "";
        }
    }

    void ttsMessage(const QString &msgSynthesis)
    {
        this->messageTTS = msgSynthesis;
    }
    void update(int msgCode)
    {
        this->messageCode = msgCode;
    }
    void update(int msgCode, const QString &message, int replyType = RT_STRING_TTS | RT_STRING_DISPLAY)
    {
        this->messageCode = msgCode;
        this->messageDisplay = message;
        this->messageTTS = message;
        this->m_replyType = replyType;
    }

    void update(int msgCode, const QString &msgDisplay, const QString &msgSynthesis, int replyType = RT_STRING_TTS | RT_STRING_DISPLAY)
    {
        this->messageCode = msgCode;
        this->messageDisplay = msgDisplay;
        this->messageTTS = msgSynthesis;
        this->m_replyType = replyType;
    }

    operator int() { return messageCode.toInt(); }

    bool isCodeValid()
    {
        return messageCode.isValid();
    }

    bool isDisplayMessageValid()
    {
        return messageDisplay.isValid();
    }

    bool isSynthesisMessageValid()
    {
        return messageTTS.isValid();
    }

    QWidget *getReplyWidget()
    {
        return m_widget;
    }

    void setReplyWidget(QWidget *w)
    {
        m_widget = w;
    }

    int getReplyType()
    {
        return m_replyType;
    }
    void  setReplyType(int type)
    {
        m_replyType = type;
    }

    int getShouldEndSession()
    {
        return m_shouldEndSession;
    }

    void setShouldEndSession(bool bShouldEndSession)
    {
        m_shouldEndSession = bShouldEndSession;
    }

private:
    QVariant messageCode;
    QVariant messageDisplay;
    QVariant messageTTS;
    QWidget     *m_widget;
    int    m_replyType;
    bool   m_shouldEndSession;
};

#endif // REPLY_H

