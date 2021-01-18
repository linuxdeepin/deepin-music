#include "voiceplugin.h"
#include <QDebug>
#include <QDBusInterface>
#include <QVariant>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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
        QDBusMessage msg  = speechbus.call(QString("invoke"), strconbine.at(0), strconbine.size() == 1 ? " " : strconbine.at(1)); //0 function  ,1 params
        qDebug() << "=============" << msg.arguments();
        //just send sth to deepin-music
        m_replyMessage = "music process message";
        if (msg.arguments().size() > 0)
            emit signaleSendMessage(msg.arguments().at(0).toString());
    } else {
        //service not start? -- start music
    }
}

QStringList VoicePlugin::analyseJsonString(const QString &str)
{
    QString jsonStr = str;
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
    if (!jsonObject.contains(QStringLiteral("semantic"))) {
        qDebug() << __FUNCTION__ << "---2---" << strlist;
        return strlist;
    }
    QJsonValue semanticValue = jsonObject.value(QStringLiteral("semantic"));
    // semantic是一个集合,不是集合直接返回空
    if (!semanticValue.isArray()) {
        qDebug() << __FUNCTION__ << "---3---" << strlist;
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
