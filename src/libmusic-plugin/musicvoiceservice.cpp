#include "musicvoiceservice.h"
#include "voiceplugin.h"

MusicVoiceService::MusicVoiceService()
{

}

QString MusicVoiceService::serviceName()
{
    return "weather";
}
int MusicVoiceService::servicePriority()
{
    return 0;
}
bool MusicVoiceService::canHandle(const QString &s)
{
    Q_UNUSED(s);
    return true;
}
IRet MusicVoiceService::service(const QString &semantic)
{
    m_voice = new VoicePlugin();
    m_voice->process(semantic);
    return ERR_SUCCESS;
}
Reply &MusicVoiceService::getServiceReply()
{
//    if (m_processResult) {
//        m_reply.setReplyType(Reply::RT_INNER_WIDGET | Reply::RT_STRING_TTS);
//        m_reply.setReplyWidget(m_widget);
//        m_reply.ttsMessage(m_widget->getTTSMessage());
//    } else {
//        m_reply.setReplyType(Reply::RT_STRING_TTS | Reply::RT_STRING_DISPLAY);
//        m_reply.ttsMessage(m_widget->getTTSMessage());
//        m_reply.displayMessage(m_widget->getTTSMessage());
//        delete  m_widget;
//        m_widget = nullptr;
//    }
    m_reply.code(ERR_SUCCESS);

    return m_reply;
}
