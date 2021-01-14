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
        QDBusMessage msg  = speechbus.call(QString("invoke"), strconbine.at(0), strconbine.at(1)); //0 function  ,1 params
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
    QStringList strlist;

    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    auto rootObject = doc.object();
    if (!(rootObject.contains("intent")
            && rootObject["intent"].isObject())) {
        return strlist;
    }

    auto intentObject = rootObject["intent"].toObject();
    if (intentObject.empty()) {
        return strlist;
    }

    QJsonArray dataArray;

    MusicVoiceMsg musicMsg;
    //analyse todo..
    return strlist;
}
