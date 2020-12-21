#include "exportedinterface.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QDebug>

#include "speechCenter.h"

ExportedInterface::ExportedInterface(QObject *parent): Dtk::Core::DUtil::DExportedInterface(parent)
{
    m_object = parent;
    m_SpeechCenter = SpeechCenter::getInstance();
}

QVariant ExportedInterface::invoke(const QString &action, const QString &parameters)const
{
//    //解析参数
//    ACTION actionInt = ACTION(action.toInt());
//    QString param1;
//    QString param2;

//    QJsonParseError json_error;
//    QJsonDocument jsonDoc(QJsonDocument::fromJson(parameters.toLocal8Bit(), &json_error));

//    if (json_error.error != QJsonParseError::NoError) {
//        return false;
//    }
//    QJsonObject rootObj = jsonDoc.object();
//    if (rootObj.contains("param1")) {
//        param1 = rootObj.value("param1").toString();
//    }
//    if (rootObj.contains("param2")) {
//        param2 = rootObj.value("param2").toString();
//    }
//    qDebug() << "-----get ExportedInterface::invoke param1:" << param1;
//    qDebug() << "-----get ExportedInterface::invoke param2:" << param2;

//    //分发语音信号
//    switch (actionInt) {
//    case ACTION::playMusic:
//        return QVariant(m_SpeechCenter->playMusic(param1));
//    case ACTION::playArtist:
//        return QVariant(m_SpeechCenter->playArtist(param1));
//    case ACTION::playArtistMusic:
//        return QVariant(m_SpeechCenter->playArtistMusic(param1, param2));
//    case ACTION::playFaverite:
//        return QVariant(m_SpeechCenter->playFaverite());
//    case ACTION::playCustom:
//        return QVariant(m_SpeechCenter->playCustom(param1));
//    case ACTION::playRadom:
//        return QVariant(m_SpeechCenter->playRadom());
//    case ACTION::pause:
//        return QVariant(m_SpeechCenter->pause());
//    case ACTION::stop:
//        return QVariant(m_SpeechCenter->stop());
//    case ACTION::resume:
//        return QVariant(m_SpeechCenter->resume());
//    case ACTION::previous:
//        return QVariant(m_SpeechCenter->previous());
//    case ACTION::next:
//        return QVariant(m_SpeechCenter->next());
//    case ACTION::favorite:
//        return QVariant(m_SpeechCenter->favorite());
//    case ACTION::unFavorite:
//        return QVariant(m_SpeechCenter->unFaverite());
//    case ACTION::setMode:
//        return QVariant(m_SpeechCenter->setMode(param1.toInt()));
//    }
    return QVariant(true);
}
