#ifndef ScheduleDemoPlugin_H
#define ScheduleDemoPlugin_H
#include <QObject>
#include "interface/systemsemanticplugin.h"
#include "scheduledemowidget.h"

#include <QMutex>
class ScheduleDemoPlugin : public QObject, public IServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(IServicePlugin)
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
public:
    explicit ScheduleDemoPlugin(QObject *parent = nullptr);
    virtual ~ScheduleDemoPlugin() override;

    virtual int init() override;
    virtual void uninit() override;

    virtual QStringList getSupportService() override;
    virtual IService *createService(const QString &service) override;
    virtual void releaseService(IService *service) override;
    virtual bool needRunInDifferentThread()  override;

public slots:
    //发送tts语音消息接口
    void slotSendMessage(QString text);

private:
    QSet<IService *> serviceSet;
    QMutex serviceLock;


};

#endif // ScheduleDemoPlugin_H
