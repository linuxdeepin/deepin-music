#include "scheduledemoplugin.h"
#include "scheduleservice.h"
ScheduleDemoPlugin::ScheduleDemoPlugin(QObject *parent)
{
    Q_UNUSED(parent);
}
ScheduleDemoPlugin::~ScheduleDemoPlugin()
{

}

int ScheduleDemoPlugin::init()
{
    return 0;
}
void ScheduleDemoPlugin::uninit()
{

}
QStringList ScheduleDemoPlugin::getSupportService()
{
    return QStringList{ "scheduleX" };
}
IService *ScheduleDemoPlugin::createService(const QString &service)
{
    if (!getSupportService().contains(service))
    {
        return nullptr;
    }
    QMutexLocker lock(&serviceLock);
    auto pService = new ScheduleService();
    connect(pService->m_weatherWidget, &ScheduleDemoWidget::signaleSendMessage, this, &ScheduleDemoPlugin::slotSendMessage);
    serviceSet.insert(pService);
    return pService;
}
//注意：一轮语义结束，则助手直接调用这个函数释放service
void ScheduleDemoPlugin::releaseService(IService* service)
{
    QMutexLocker lock(&serviceLock);
    for (auto iter = serviceSet.begin(); iter != serviceSet.end();iter++)
    {
        if(service && service == *iter)
        {
            serviceSet.erase(iter);
            break;
        }
    }
    if(service)
    {
        delete service;
    }
}

void ScheduleDemoPlugin::slotSendMessage(QString text)
{
    qDebug()<<__FUNCTION__<<text;
    Reply reply;
    reply.setReplyType(Reply::RT_STRING_TTS | Reply::RT_STRING_DISPLAY);
    reply.ttsMessage(text);
    reply.displayMessage(text);
    if(this->m_messageHandle)
        this->m_messageHandle(this, reply);
}

bool ScheduleDemoPlugin::needRunInDifferentThread()
{
    return false;
}

