#ifndef ScheduleDemoWidget_H
#define ScheduleDemoWidget_H

#include <DWidget>
#include "interface/reply.h"
DWIDGET_USE_NAMESPACE

class ScheduleDemoWidget: public QWidget
{
    Q_OBJECT
public:
    ScheduleDemoWidget(QWidget *parent = nullptr);

    void process(const QString &semantic);

public  slots:
    void slotReceivce(QVariant data, Reply *reply);

public:

    QString m_ttsMessage;

    QString m_displyMessage;
signals:

    void signaleSendMessage(QString text);
};

#endif // ScheduleDemoWidget_H
