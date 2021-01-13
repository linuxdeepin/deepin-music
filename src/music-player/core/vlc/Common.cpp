
#include "Common.h"

QStringList VlcCommon::args()
{
    QStringList args_list;

    QString args = qgetenv("VLC_ARGS");
    if (!args.isEmpty())
        args_list << args.split(" ", QString::SkipEmptyParts);
    else {
        args_list << "--intf=dummy"
                  << "--no-media-library"
                  << "--no-stats"
                  << "--no-osd"
                  << "--no-loop"
                  << "--no-video-title-show"
                  << "--drop-late-frames";
    }

    return args_list;
}

//bool VlcCommon::setPluginPath(const QString &path)
//{
//    if (qgetenv("VLC_PLUGIN_PATH").isEmpty()) {
//        return qputenv("VLC_PLUGIN_PATH", path.toLocal8Bit());
//    }

//    return false;
//}
