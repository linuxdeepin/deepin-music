#include "scheduleservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ScheduleService::ScheduleService(): m_weatherWidget(new ScheduleDemoWidget)
{

}

QString ScheduleService::serviceName()
{
    return "scheduleX";
}
int ScheduleService::servicePriority()
{
    return 0;
}
bool ScheduleService::canHandle(const QString &s)
{
    Q_UNUSED(s);
    return true;
}
IRet ScheduleService::service(const QString &semantic)
{
    qDebug() << "云端返回的语义:" << semantic;
    //解析云端返回的数据,进行业务处理
    QString ttsMsg; //返回给语音助手播报的数据
    QJsonDocument doc = QJsonDocument::fromJson(semantic.toUtf8());
    auto rootObject = doc.object();
    if (!(rootObject.contains("intent")
            && rootObject["intent"].isObject())) {
        return ERR_FAIL;
    }

    auto intentObject = rootObject["intent"].toObject();
    if (intentObject.empty()) {
        return ERR_FAIL;
    }

    if (intentObject.contains("voice_answer") && intentObject["voice_answer"].isArray()) {
        auto voiceAnsObjArr = intentObject["voice_answer"].toArray();
        for (int i = 0; i < voiceAnsObjArr.size(); ++i) {
            auto voiceAnsObj = voiceAnsObjArr[i].toObject();
            if (voiceAnsObj.contains("type")  && voiceAnsObj["type"].isString()) {
                if (voiceAnsObj["type"] != "TTS") {
                    continue;
                }
                if (voiceAnsObj.contains("content") && voiceAnsObj["content"].isString()) {
                    //语音播报的文本
                    ttsMsg = voiceAnsObj["content"].toString();
                    break;
                }
            }
        }
    }

    m_weatherWidget->m_ttsMessage = ttsMsg;
    m_weatherWidget->m_displyMessage = ttsMsg;
    //生成界面
    m_weatherWidget->process(semantic);

    return ERR_SUCCESS;
}

Reply &ScheduleService::getServiceReply()
{
    bool flag = true;
    //传递界面给助手展示
    if (flag) {
        m_reply.setReplyType(Reply::RT_INNER_WIDGET | Reply::RT_STRING_TTS | Reply::RT_STRING_DISPLAY);
        m_reply.setReplyWidget(m_weatherWidget);
        m_reply.ttsMessage(m_weatherWidget->m_ttsMessage);
        m_reply.displayMessage(m_weatherWidget->m_displyMessage);
    }
    //传递文本给助手展示
    else {
        m_reply.setReplyType(Reply::RT_STRING_TTS | Reply::RT_STRING_DISPLAY);
        m_reply.ttsMessage(m_weatherWidget->m_ttsMessage);
        m_reply.displayMessage(m_weatherWidget->m_displyMessage);
    }
    return m_reply;
}
