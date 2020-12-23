#ifndef ScheduleService_H
#define ScheduleService_H

#include "interface/service.h"
#include "scheduledemowidget.h"

class ScheduleService : public IService
{
    Q_OBJECT
public:
    ScheduleService();
    QString serviceName() override;
    int servicePriority() override;
    bool canHandle(const QString& s) override;
    IRet service(const QString& semantic) override;
    Reply& getServiceReply() override;

signals:

    void signalSendMessage(const QVariant &data, Reply*  reply);

private:
    Reply                   m_reply;
    bool                    m_processResult = false;
public:
    ScheduleDemoWidget*      m_weatherWidget;


};

#endif // ScheduleService_H
