

#include "Enums.h"

Vlc::Vlc(QObject *parent)
    : QObject(parent) {}

QStringList Vlc::logLevel()
{
    QStringList list;
    list << "debug"
         << "debug"
         << "notice"
         << "warning"
         << "error"
         << "disabled";

    return list;
}

QStringList Vlc::audioCodec()
{
    QStringList list;
    list << "none"
         << "mpga"
         << "mp3"
         << "mp4a"
         << "vorb"
         << "flac";

    return list;
}

QStringList Vlc::audioOutput()
{
    QStringList list;
    list << "default";

    return list;
}

QStringList Vlc::audioOutputHuman()
{
    QStringList list;
    list << "default";

    return list;
}

QStringList Vlc::deinterlacing()
{
    QStringList list;
    list << ""
         << "discard"
         << "blend"
         << "man"
         << "bob"
         << "linear"
         << "x"
         << "yadif"
         << "yadif2x"
         << "phosphor"
         << "ivtc";

    return list;
}

QStringList Vlc::mux()
{
    QStringList list;
    list << "ts"
         << "ps"
         << "mp4"
         << "ogg"
         << "avi";

    return list;
}

QStringList Vlc::ratio()
{
    QStringList list;
    list << ""
         << "ignore"
         << "16:9"
         << "16:10"
         << "185:100"
         << "221:100"
         << "235:100"
         << "239:100"
         << "4:3"
         << "5:4"
         << "5:3"
         << "1:1";

    return list;
}

QStringList Vlc::ratioHuman()
{
    QStringList list;
    list << ""
         << ""
         << "16:9"
         << "16:10"
         << "1.85:1"
         << "2.21:1"
         << "2.35:1"
         << "2.39:1"
         << "4:3"
         << "5:4"
         << "5:3"
         << "1:1";

    return list;
}

QSizeF Vlc::ratioSize(const Vlc::Ratio &ratio)
{
    switch (ratio) {
    case Vlc::R_16_9:
        return QSizeF(16, 9);
    case Vlc::R_16_10:
        return QSizeF(16, 10);
    case Vlc::R_185_100:
        return QSizeF(185, 100);
    case Vlc::R_221_100:
        return QSizeF(221, 100);
    case Vlc::R_235_100:
        return QSizeF(235, 100);
    case Vlc::R_239_100:
        return QSizeF(239, 100);
    case Vlc::R_4_3:
        return QSizeF(4, 3);
    case Vlc::R_5_4:
        return QSizeF(5, 4);
    case Vlc::R_5_3:
        return QSizeF(5, 3);
    case Vlc::R_1_1:
        return QSizeF(1, 1);
    default:
        return QSizeF(0, 0);
    }
}

QList<float> Vlc::scale()
{
    QList<float> list;
    list << 0.0f
         << 1.05f
         << 1.1f
         << 1.2f
         << 1.3f
         << 1.4f
         << 1.5f
         << 1.6f
         << 1.7f
         << 1.8f
         << 1.9f
         << 2.0f;

    return list;
}

QStringList Vlc::videoCodec()
{
    QStringList list;
    list << "none"
         << "mpgv"
         << "mp4v"
         << "h264"
         << "theora";

    return list;
}

QStringList Vlc::videoOutput()
{
    QStringList list;
    list << "default";

#if defined(Q_OS_LINUX)
    list << "x11"
         << "xvideo"
         << "glx";
#elif defined(Q_OS_WIN32)
    list << "directx"
         << "direct3d"
         << "opengl";
#endif

    return list;
}

QStringList Vlc::videoOutputHuman()
{
    QStringList list;
    list << "default";

#if defined(Q_OS_LINUX)
    list << "X11"
         << "XVideo"
         << "OpenGL";
#elif defined(Q_OS_WIN32)
    list << "DirectX"
         << "Direct3D"
         << "OpenGL";
#endif

    return list;
}
