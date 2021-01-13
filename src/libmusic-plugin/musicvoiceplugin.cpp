#include "musicvoiceplugin.h"
#include "musicvoiceservice.h"
MusicVoicePlugin::MusicVoicePlugin(QObject *parent)
{
    Q_UNUSED(parent);
}
MusicVoicePlugin::~MusicVoicePlugin()
{

}

int MusicVoicePlugin::init()
{
    return 0;
}
void MusicVoicePlugin::uninit()
{

}
QStringList MusicVoicePlugin::getSupportService()
{
    return QStringList{ "weather" };
}
IService *MusicVoicePlugin::createService(const QString &service)
{
    if (!getSupportService().contains(service)) {
        return nullptr;
    }
    QMutexLocker lock(&serviceLock);
    auto pService = new MusicVoiceService();
    serviceSet.insert(pService);
    return pService;
}
void MusicVoicePlugin::releaseService(IService *service)
{
    QMutexLocker lock(&serviceLock);
    for (auto iter = serviceSet.begin(); iter != serviceSet.end(); iter++) {
        if (service && service == *iter) {
            serviceSet.erase(iter);
            break;
        }
    }
    if (service) {
        delete service;
    }
}

bool MusicVoicePlugin::needRunInDifferentThread()
{
    return false;
}
