

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include "../util/global.h"
#include <vlc/vlc.h>
#include "Enums.h"
#include "Error.h"
#include "Instance.h"




void logCallback(void *data,
                 int level,
                 const libvlc_log_t *ctx,
                 const char *fmt,
                 va_list args)
{
    Q_UNUSED(ctx)

    VlcInstance *instance = static_cast<VlcInstance *>(data);
    if (instance->logLevel() > level) {
        return;
    }

    char *result;
    if (vasprintf(&result, fmt, args) < 0) {
        return; // LCOV_EXCL_LINE
    }

    QString message(result);
    free(result);

    message.prepend("VlcInstance  libvlc: ");

    switch (level) {
    case Vlc::ErrorLevel:
        qCritical(message.toUtf8().data(), NULL);
        break;
    case Vlc::WarningLevel:
        qWarning(message.toUtf8().data(), NULL);
        break;
    case Vlc::NoticeLevel:
    case Vlc::DebugLevel:
    default:
        qDebug(message.toUtf8().data(), NULL);
        break;
    }
}

VlcInstance::VlcInstance(const QStringList &args,
                         QObject *parent)
    : QObject(parent),
      _vlcInstance(nullptr),
      _status(false),
      _logLevel(Vlc::ErrorLevel)
{
    Q_UNUSED(args)
// Convert arguments to required format

//    char *argv[args.count()];
//    for (int i = 0; i < args.count(); ++i)
//        argv[i] = (char *)qstrdup(args.at(i).toUtf8().data());

    // Create new libvlc instance
//    _vlcInstance = libvlc_new(args.count(), argv);
    _vlcInstance = libvlc_new(0, nullptr);
    libvlc_set_user_agent(_vlcInstance, Global::getAppName().toStdString().c_str(), "");//name
    libvlc_set_app_id(_vlcInstance, "", "", "deepin-music");//icon

    qRegisterMetaType<Vlc::Meta>("Vlc::Meta");
    qRegisterMetaType<Vlc::State>("Vlc::State");

    VlcError::showErrmsg();

    // Check if instance is running
    if (_vlcInstance) {
        libvlc_log_set(_vlcInstance, logCallback, this);
        _status = true;
        qDebug() << "Using libvlc version:" << version();
    } else {
        qCritical() << "VLC Error: libvlc failed to load!";
    }
}

VlcInstance::~VlcInstance()
{
    if (_status && _vlcInstance) {
        libvlc_release(_vlcInstance);
    }
}

libvlc_instance_t *VlcInstance::core()
{
    return _vlcInstance;
}

bool VlcInstance::status() const
{
    return _status;
}

Vlc::LogLevel VlcInstance::logLevel() const
{
    return _logLevel;
}

void VlcInstance::setLogLevel(Vlc::LogLevel level)
{
    _logLevel = level;
}

//QString VlcInstance::libVersion()
//{
//    QString version;
//#if defined(LIBVLCQT_VERSION)
//    version.append(QString(LIBVLCQT_VERSION));
//#else
//    version.append(QString("Unknown"));
//#endif //LIBVLCQT_VERSION

//#if defined(LIBVLCQT_VERSION_VCS)
//    if (QString(LIBVLCQT_VERSION_VCS) != "0" && QString(LIBVLCQT_VERSION_VCS) != "") {
//        version.append("-" + QString(LIBVLCQT_VERSION_VCS));
//    }
//#endif //LIBVLCQT_VERSION

//    return version;
//}

//int VlcInstance::libVersionMajor()
//{
//    int version = -1;
//#if defined(LIBVLCQT_VERSION_MAJOR)
//    version = LIBVLCQT_VERSION_MAJOR;
//#endif //LIBVLCQT_VERSION

//    return version;
//}

//int VlcInstance::libVersionMinor()
//{
//    int version = -1;
//#if defined(LIBVLCQT_VERSION_MINOR)
//    version = LIBVLCQT_VERSION_MINOR;
//#endif //LIBVLCQT_VERSION

//    return version;
//}

QString VlcInstance::changeset()
{
    // Returns libvlc changeset
    return QString(libvlc_get_changeset());
}

QString VlcInstance::compiler()
{
    // Returns libvlc compiler version
    return QString(libvlc_get_compiler());
}

QString VlcInstance::version()
{
    // Returns libvlc version
    return QString(libvlc_get_version());
}


//void VlcInstance::setAppId(const QString &id,
//                           const QString &version,
//                           const QString &icon)
//{
//    libvlc_set_app_id(_vlcInstance, id.toUtf8().data(), version.toUtf8().data(), icon.toUtf8().data());
//}

//QList<VlcModuleDescription *> VlcInstance::audioFilterList() const
//{
//    libvlc_module_description_t *original = libvlc_audio_filter_list_get(_vlcInstance);
//    if (original == NULL) {
//        return QList<VlcModuleDescription *>(); // LCOV_EXCL_LINE
//    }

//    libvlc_module_description_t *list = original;
//    QList<VlcModuleDescription *> audioFilters;
//    do {
//        VlcModuleDescription *module = new VlcModuleDescription(VlcModuleDescription::AudioFilter, list->psz_name);
//        module->setLongName(list->psz_longname);
//        module->setShortName(list->psz_shortname);
//        module->setHelp(list->psz_help);
//        audioFilters << module;

//        list = list->p_next;
//    } while (list->p_next);

//    libvlc_module_description_list_release(original);

//    return audioFilters;
//}

//QList<VlcModuleDescription *> VlcInstance::videoFilterList() const
//{
//    libvlc_module_description_t *original = libvlc_video_filter_list_get(_vlcInstance);
//    if (original == NULL) {
//        return QList<VlcModuleDescription *>(); // LCOV_EXCL_LINE
//    }

//    libvlc_module_description_t *list = original;
//    QList<VlcModuleDescription *> videoFilters;
//    do {
//        VlcModuleDescription *module = new VlcModuleDescription(VlcModuleDescription::VideoFilter, list->psz_name);
//        module->setLongName(list->psz_longname);
//        module->setShortName(list->psz_shortname);
//        module->setHelp(list->psz_help);
//        videoFilters << module;

//        list = list->p_next;
//    } while (list->p_next);

//    libvlc_module_description_list_release(original);

//    return videoFilters;
//}
