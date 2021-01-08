#include "voiceplugin.h"
#include <QDebug>
#include <QDBusInterface>
#include <QVariant>

VoicePlugin::VoicePlugin(QObject *parent): QObject(parent)
{

}

//void VoicePlugin::slotReceivce(QVariant data, Reply *reply)
//{
//    //do nothing
//}

void VoicePlugin::process(const QString &semantic)
{
    QString strconbine = semantic;
    QDBusInterface speechbus("org.mpris.MediaPlayer2.DeepinMusic",
                             "/org/mpris/speech",
                             "com.deepin.speech",
                             QDBusConnection::sessionBus());
    if (speechbus.isValid()) {
        //send sth
        QDBusMessage msg  = speechbus.call(QString("invoke"), "next", "");
        qDebug() << "=============" << msg.arguments();
        //just send sth to deepin-music
        m_replyMessage = "1111111111111";
        emit signaleSendMessage(msg.arguments().at(0).toString());

    } else {
        //service not start? -- start music
    }
}
