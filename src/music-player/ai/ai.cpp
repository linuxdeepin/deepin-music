// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ai.h"
#include "speechCenter.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QtDBus>


UosAIInterface::UosAIInterface(QObject *parent)
    : QObject(parent)
{
    if (!m_isCopilotConnected) {
        m_aiSessionBus = new QDBusInterface("com.deepin.copilot", "/com/deepin/copilot", "com.deepin.copilot", QDBusConnection::sessionBus());

        if (m_aiSessionBus->isValid())
            m_isCopilotConnected = true;
    }
}

void UosAIInterface::parseAIFunction(/*QByteArray functions*/)
{
    QDBusReply<QString> functions = m_aiSessionBus->call("cachedFunctions");
    qDebug() << __func__ << functions;
//    QByteArray arr = "{\"functions\":[{\"arguments\":\"{\\\"name\\\":\\\"精卫\\\"}\",\"name\":\"playOneSong\"}]}";
//    QJsonDocument jsonDocu = QJsonDocument::fromJson(arr);
    QJsonDocument jsonDocu = QJsonDocument::fromJson(functions.value().toUtf8());

    if (jsonDocu.isObject()) {
        QJsonObject objRoot = jsonDocu.object();

        for (QString key : objRoot.keys()) {
            QJsonValue valueRoot = objRoot.value(key);

            if (valueRoot.isArray() && key == "functions") {
                QJsonArray array = valueRoot.toArray();

                for (int i = 0; i < array.count(); ++i) {
                    if (array[i].isObject()) {
                        //解析每个function的名称和参数
                        QJsonObject funcObj = array[i].toObject();
                        QString functionName = nullptr;
                        QMap<QString, QString>functionArguments;

                        for (QString funcKey : funcObj.keys()) {
                            if (funcKey == "name") {
                                functionName = funcObj[funcKey].toString();
                                qDebug() << "function name: " << functionName;
                            } else if (funcKey == "arguments") {
                                QByteArray arr = funcObj[funcKey].toString().toUtf8();
                                QJsonDocument argDoc = QJsonDocument::fromJson(arr);

                                if (argDoc.isObject()) {
                                    QJsonObject argObj= argDoc.object();

                                    for (QString argKey : argObj.keys()) {
                                        if (argObj[argKey].isDouble()) {
                                            functionArguments[argKey] = QString::number(argObj[argKey].toInt());
                                        } else {
                                            functionArguments[argKey] = argObj[argKey].toString();
                                        }
                                        qDebug() << "function argument:  " << argKey << ": " << functionArguments[argKey];
                                    }
                                }
                            }
                        }

                        handleAICall(functionName, functionArguments);


                    } else if (array.at(i).isString()) {
                        qDebug() << key <<": "<< array.at(i).toString();
                    }
                }

            } else if (valueRoot.isString()) {
                qDebug() << key <<": "<< valueRoot.toString();
            } else if (valueRoot.isDouble()) {
                qDebug() << key <<": "<< valueRoot.toInt();
            } else if (valueRoot.isObject()) {
                qDebug() << key <<": "<< valueRoot.toObject();
            }
        }
    }
}

void UosAIInterface::handleAICall(QString &funcName, QMap<QString, QString> &arguments)
{
    QStringList keys = arguments.keys();
    QDBusInterface speechbus("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/speech",
                             "com.deepin.speech",
                             QDBusConnection::sessionBus());
    if (!speechbus.isValid()) {
        qDebug() << __func__ << "first started.";
        if (funcName == "playOneSong" && keys.size() == 1) {
            if (keys[0] == "name") {
                QTimer::singleShot(1500, this, [=](){ //延时，保证状态正常更新
                    SpeechCenter::getInstance()->playMusic(arguments[keys[0]]);
                });
            } else if (keys[0] == "album") {
                QTimer::singleShot(500, this, [=](){
                    SpeechCenter::getInstance()->playAlbum(arguments[keys[0]]);
                });
            } else if (keys[0] == "songList") {
                QTimer::singleShot(500, this, [=](){
                    SpeechCenter::getInstance()->playSonglist(arguments[keys[0]]);
                });
            } else if (keys[0] == "singer") {
                QTimer::singleShot(500, this, [=](){
                    SpeechCenter::getInstance()->playArtist(arguments[keys[0]]);
                });
            }
        } else if (funcName == "playOneSong" && keys.size() == 2) {
            QString singer, title;
            for(QString key : keys) {
                if (key == "singer") {
                    singer = arguments[key];
                } else if (key == "name") {
                    title = arguments[key];
                }
            }
            QTimer::singleShot(500, this, [=](){
                SpeechCenter::getInstance()->playArtistMusic(singer + ":" + title);
            });
        } else if (funcName == "playMyFavorite") {
            if (keys.size() == 0) {
                QTimer::singleShot(500, this, [=](){
                    SpeechCenter::getInstance()->playFaverite("");
                });
            }
        }
        return;
    }

    if (funcName == "playOneSong") {
        switch (keys.size()) {
        case 0: {
            //播放任意歌曲
            QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playMusic", "");
            break;
        }

        case 1: {
            if (keys[0] == "name") {
                QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playMusic", arguments[keys[0]]);
            } else if (keys[0] == "album") {
                QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playAlbum", arguments[keys[0]]);
            } else if (keys[0] == "singer") {
                QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playArtist", arguments[keys[0]]);
            } else if (keys[0] == "songList") {
                QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playSonglist", arguments[keys[0]]);
            } else if (keys[0] == "index") {
                QRegExp rx("(\\d+)");   //保证下标只有数字
                int pos = 0;
                QString index;

                while((pos = rx.indexIn(arguments[keys[0]], pos)) != -1) {
                    index = rx.cap(1);
                    pos += rx.matchedLength();
                }

                if (index.toInt() == 0)
                    index = QString::number(index.toInt() + 1);

                if (!index.isEmpty() && index.toInt() > 0)
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playIndex", index);
            }
            break;
        }

        case 2: {
            QString singer, title;
            for(QString key : keys) {
                if (key == "singer") {
                    singer = arguments[key];
                } else if (key == "name") {
                    title = arguments[key];
                }
            }
            QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playArtistMusic", singer + ":" + title);
            break;
        }

        default:
            break;
        }

    } else if (funcName == "stateControl") {
        switch (keys.size()) {
        case 0:
            break;

        case 1: {
            if (keys[0] == "control") {
                QString playMode;

                if (arguments[keys[0]] == "Play") {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "resume", "");
                } else if (arguments[keys[0]] == "Pause") {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "pause", "");
                } else if (arguments[keys[0]] == "Stop") {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "stop", "");
                } else if (arguments[keys[0]] == "Previous") {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "pre", "");
                } else if (arguments[keys[0]] == "Next") {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "next", "");
                } else if (arguments[keys[0]].contains("ListLoop")) {
                    playMode = "0";
                } else if (arguments[keys[0]].contains("SingleLoop")) {
                    playMode = "1";
                } else if (arguments[keys[0]].contains("Shuffle")) {
                    playMode = "2";
                }

                if (!playMode.isEmpty()) {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "setMode", QString(playMode));
                }
            }

            break;
        }

        default:
            break;
        }
    } else if (funcName == "seek") {
        switch (keys.size()) {
        case 0:
            break;

        case 1: {
            if (keys[0] == "position") {
                QRegExp rx("(\\d+)");
                int pos = 0;
                QStringList times;

                while((pos = rx.indexIn(arguments[keys[0]], pos)) != -1) {
                    times.append( rx.cap(1));
                    pos += rx.matchedLength();
                }
                if (times.isEmpty()) {
                    return;
                } else if(times.size() == 1) {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "setPosition", QString::number(times[0].toInt() * 1000));
                } else if(times.size() == 2) {
                    QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "setPosition", QString::number((times[0].toInt() * 60 + times[1].toInt()) * 1000));
                }
            }

            break;
        }

        default:
            break;
        }

    } else if (funcName == "addRemoveFavorite") {
        switch (keys.size()) {
        case 0:
            break;

        case 1: {
            if (keys[0] == "addFavorite") {
                QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "addFaverite", "");
            } else if (keys[0] == "removeFavorite") {
                QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "removeFaverite", "");
            }

            break;
        }
        default:
            break;

        }
    } else if (funcName == "playMyFavorite") {
        switch (keys.size()) {
        case 0: {
            QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), "playFaverite", "");
            break;
        }

        default:
            break;

        }
    }
}
