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

#ifndef VoicePlugin_H
#define VoicePlugin_H

#include <QObject>
#include <QVector>

#include <DSettings>

class VoicePlugin: public QObject
{
    Q_OBJECT
public:

    struct MusicContent {
        MusicContent() : name(""), value("")
        {
        }

        bool operator ==(const MusicContent &content)
        {
            name = content.name;
            value = content.value;
            return true;
        }
        QString name;
        QString value;
    } ;

    struct MusicVoiceMsg {
        QString intent; //执行的操作
        QVector<MusicContent> contents;//key-name
    };

    explicit VoicePlugin(QObject *parent = nullptr);

    void process(const QString &semantic);

public  slots:

public:
    QString m_ttsMessage;
    QString m_replyMessage;
signals:
    void signaleSendMessage(QString text);
private:
    QStringList analyseJsonString(const QString &semantic);
    Dtk::Core::DSettings *m_settings = nullptr;
};

#endif // VoicePlugin_H
