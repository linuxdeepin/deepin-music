#ifndef ScheduleDemoWidget_H
#define ScheduleDemoWidget_H

#include <QObject>
class VoicePlugin: public QObject
{
    Q_OBJECT
public:
    VoicePlugin(QObject *parent = nullptr);

    void process(const QString &semantic);

public  slots:
    //void slotReceivce(QVariant data, Reply *reply);

public:
    QString m_ttsMessage;

    QString m_replyMessage;
signals:
    void signaleSendMessage(QString text);
};

#endif // ScheduleDemoWidget_H
