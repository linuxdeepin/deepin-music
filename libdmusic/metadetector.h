#pragma once

#include "util/singleton.h"

#include <mediameta.h>

class QFileInfo;
class LIBDMUSICSHARED_EXPORT MetaDetector: public DMusic::DSingleton<MetaDetector>
{
public:
    MetaDetector();

    static void updateMetaFromLocalfile(MediaMeta *meta, const QFileInfo &fileInfo);
};
