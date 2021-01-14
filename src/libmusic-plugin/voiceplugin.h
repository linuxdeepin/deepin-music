#ifndef ScheduleDemoWidget_H
#define ScheduleDemoWidget_H

#include <QObject>
#include <QVector>

class VoicePlugin: public QObject
{
    Q_OBJECT
public:

    struct MusicContent {
        MusicContent()
        {
            name = "";
            value = "";
        }

        bool operator ==(const MusicContent &content)
        {
            name = content.name;
            value = content.value;
            return true;
        }
        QString name;
        QString value;
    } ;

    struct MusicVoiceMsg {
        QString intent; //执行的操作
        QVector<MusicContent> contents;//key-name
    };

    VoicePlugin(QObject *parent = nullptr);

    void process(const QString &semantic);

public  slots:

public:
    QString m_ttsMessage;

    QString m_replyMessage;
signals:
    void signaleSendMessage(QString text);
private:
    QStringList analyseJsonString(const QString &str);
};

#endif // ScheduleDemoWidget_H
