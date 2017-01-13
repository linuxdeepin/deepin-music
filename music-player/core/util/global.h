#pragma once

#include <util/singleton.h>
#include <QString>

class Global : public DMusic::DSingleton<Global>
{
public:
    static QString configPath();
    static QString cacheDir();

private:
    friend class DMusic::DSingleton<Global>;
};
