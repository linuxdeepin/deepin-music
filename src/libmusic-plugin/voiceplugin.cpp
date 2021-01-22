#include "voiceplugin.h"
#include <QDebug>
#include <QDBusInterface>
#include <QVariant>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDBusReply>
#include <QThread>
#include <QDir>

VoicePlugin::VoicePlugin(QObject *parent): QObject(parent)
{

}

void VoicePlugin::process(const QString &semantic)
{
    QStringList strconbine = analyseJsonString(semantic);
    if (strconbine.size() < 1)
        return;
    QDBusInterface speechbus("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/speech",
                             "com.deepin.speech",
                             QDBusConnection::sessionBus());
    if (speechbus.isValid()) {
        //send sth
        QDBusReply<QVariant> msg  = speechbus.call(QString("invoke"), strconbine.at(0), strconbine.size() == 1 ? " " : strconbine.at(1)); //0 function  ,1 params
        //just send sth to deepin-music
        if (msg.isValid()) {
            m_replyMessage = msg.value().toString();
            m_ttsMessage = msg.value().toString();
            emit signaleSendMessage(m_ttsMessage);
        }
    } else {
        // 启动音乐
        QDBusInterface startbus("com.deepin.SessionManager",
                                "/com/deepin/StartManager",
                                "com.deepin.StartManager",
                                QDBusConnection::sessionBus(),
                                this);
        if (startbus.isValid()) {
            QList<QVariant> strlist;
            strlist << "/usr/share/applications/deepin-music.desktop";
            QDBusReply<QVariant> msg  = startbus.asyncCallWithArgumentList(QString("Launch"), strlist);
            //just send sth to deepin-music
            int checkCount = 0;
            while (checkCount <= 20) {
                QDBusInterface speechBus("org.mpris.MediaPlayer2.DeepinMusic",
                                         "/org/mpris/speech",
                                         "com.deepin.speech",
                                         QDBusConnection::sessionBus());
                if (speechBus.isValid()) {
                    break;
                }
                checkCount++;
                QThread::msleep(100);
            }
            // 音乐刚启动，添加延时给音乐初始化的时间,考虑到有特殊歌曲加载进度条时间较长，延时给到1s
            QThread::msleep(1000);
            QDBusInterface speechBus("org.mpris.MediaPlayer2.DeepinMusic",
                                     "/org/mpris/speech",
                                     "com.deepin.speech",
                                     QDBusConnection::sessionBus());
            if (speechBus.isValid()) {
                QDBusReply<QVariant> message  = speechBus.call(QString("invoke"), strconbine.at(0), strconbine.size() == 1 ? " " : strconbine.at(1)); //0 function  ,1 params
                //just send sth to deepin-music
                if (message.isValid()) {
                    m_replyMessage = message.value().toString();
                    m_ttsMessage = message.value().toString();
                    emit signaleSendMessage(m_ttsMessage);
                }
            }
        }
    }
}

QStringList VoicePlugin::analyseJsonString(const QString &semantic)
{
    QString jsonStr = semantic;
//    QMessageBox::about(nullptr, "1", jsonStr);
    QStringList strlist;

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseJsonErr);
    if (!(parseJsonErr.error == QJsonParseError::NoError)) {
        qDebug() << tr("解析json文件错误！");
        qDebug() << __FUNCTION__ << "---1---" << strlist;
        return strlist;
    }
    QJsonObject jsonObject = document.object();
    // 找到semantic
    QString function;
    QString artist;
    QString song;
    QString source;
    QString sourceType;
    QString insType;
    QString genre;
    // 不包含semantic，直接返回空
    if (!jsonObject.contains(QStringLiteral("intent"))) {
        return strlist;
    }
    QJsonValue intentValue = jsonObject.value(QStringLiteral("intent"));
    QJsonObject intentObject = intentValue.toObject();
    // 不包含semantic，直接返回空
    if (!intentObject.contains(QStringLiteral("semantic"))) {
        return strlist;
    }
    QJsonValue semanticValue = intentObject.value(QStringLiteral("semantic"));
    // semantic是一个集合,不是集合直接返回空
    if (!semanticValue.isArray()) {
        return strlist;
    }
    QJsonArray semanticArray = semanticValue.toArray();
    qDebug() << __FUNCTION__ << "semanticArray.size() = " << semanticArray.size();
    for (int i = 0; i < semanticArray.size(); i++) {
        QJsonValue commandValue = semanticArray.at(i);
        QJsonObject commandObject = commandValue.toObject();
        // 遍历找到集合中的intent并获取内容
        if (commandObject.contains(QStringLiteral("intent"))) {
            function = commandObject["intent"].toString();
        }
        if (commandObject.contains(QStringLiteral("slots"))) {
            QJsonValue slotsValue = commandObject["slots"];
            QJsonObject slotsObject = slotsValue.toObject();
            // 找到集合slots,不是集合继续执行
            if (!slotsValue.isArray()) {
                continue;
            }
            QJsonArray slotsArray = slotsValue.toArray();
            // 遍历集合中的内容
            qDebug() << __FUNCTION__ << "slotsArray.size() = " << slotsArray.size();
            for (int j = 0; j < slotsArray.size(); j++) {
                QJsonValue slotsArrayValue = slotsArray.at(j);
                QJsonObject slotsArrayObject = slotsArrayValue.toObject();
                if (!slotsArrayObject.contains(QStringLiteral("name"))) {
                    continue;
                }
                qDebug() << __FUNCTION__ << "name = " << slotsArrayObject["name"].toString();
                if (slotsArrayObject["name"].toString() == "artist") {
                    if (slotsArrayObject.contains(QStringLiteral("value"))) {
                        artist = slotsArrayObject["value"].toString();
                    }
                }
                if (slotsArrayObject["name"].toString() == "song") {
                    if (slotsArrayObject.contains(QStringLiteral("value"))) {
                        song = slotsArrayObject["value"].toString();
                    }
                }
                if (slotsArrayObject["name"].toString() == "source") {
                    if (slotsArrayObject.contains(QStringLiteral("value"))) {
                        source = slotsArrayObject["value"].toString();
                    }
                }
                if (slotsArrayObject["name"].toString() == "sourceType") {
                    if (slotsArrayObject.contains(QStringLiteral("value"))) {
                        sourceType = slotsArrayObject["value"].toString();
                    }
                }
                if (slotsArrayObject["name"].toString() == "insType") {
                    if (slotsArrayObject.contains(QStringLiteral("value"))) {
                        insType = slotsArrayObject["value"].toString();
                    }
                }
                if (slotsArrayObject["name"].toString() == "genre") {
                    if (slotsArrayObject.contains(QStringLiteral("value"))) {
                        genre = slotsArrayObject["value"].toString();
                    }
                }
            }
        }
    }

    if (function == "PLAY") {
        if (!genre.isEmpty()) {
            strlist << "playMusic";
        } else if (!artist.isEmpty() && !song.isEmpty()) {
            strlist << "playArtistMusic" << (artist + ":" + song);
        } else if (!artist.isEmpty() && song.isEmpty()) {
            strlist << "playArtist" << artist;
        } else if (artist.isEmpty() && !song.isEmpty()) {
            strlist << "playMusic" << song;
        } else if (sourceType == "自定义") {
            strlist << "playSonglist";
        } else if (sourceType == "专辑" && !source.isEmpty()) {
            strlist << "playAlbum" << source;
        }
    }  else if (!song.isEmpty() && function == "RANDOM_SEARCH") {
        strlist << "playMusic" << song;
    } else if (song.isEmpty() && function == "RANDOM_SEARCH") {
        strlist << "playMusic";
    } else if (!sourceType.isEmpty() && function == "PLAY") {
        if (sourceType == "专辑" && !source.isEmpty()) {
            strlist << "playAlbum" << source;
        }
    } else if (function == "INSTRUCTION") {
        if (sourceType == "歌单" && source == "我的收藏" && insType == "play") {
            strlist << "playFaverite" << "fav";
        } else if (sourceType == "歌单" && source == "我的收藏" && insType == "insert") {
            strlist << "addFaverite";
        } else if (sourceType == "歌单" && source == "我的收藏" && insType == "delete") {
            strlist << "removeFaverite";
        } else if (insType == "pause") {
            strlist << "pause";
        } else if (insType == "replay") {
            strlist << "resume";
        } else if (insType == "past") {
            strlist << "pre";
        } else if (insType == "next") {
            strlist << "next";
        } else if (insType == "cycle") {
            strlist << "setMode" << "1";
        } else if (insType == "loop") {
            strlist << "setMode" << "0";
        } else if (insType == "random") {
            strlist << "setMode" << "2";
        }
    }
    return strlist;
}
