#ifndef WEATHERPLUGIN_H
#define WEATHERPLUGIN_H
#include "interface/service.h"
#include "systemsemanticplugin.h"
#include <QMutex>
class MusicVoicePlugin : public QObject,public IServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(IServicePlugin)
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
public:
    explicit MusicVoicePlugin(QObject *parent = nullptr);
    virtual ~MusicVoicePlugin() override;

    virtual int init() override;
    virtual void uninit() override;

    virtual QStringList getSupportService() override;
    virtual IService *createService(const QString &service) override;
    virtual void releaseService(IService* service) override;
    virtual bool needRunInDifferentThread()  override;

private:
    QSet<IService *> serviceSet;
    QMutex serviceLock;
};


#endif // WEATHERPLUGIN_H
