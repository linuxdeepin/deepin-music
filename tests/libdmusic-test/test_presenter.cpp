#include <QTest>

#include <unistd.h>
#include <gtest/gtest.h>

#include "presenter.h"

TEST(Presenter, Presenter)
{
    Presenter * presenter = new Presenter("unknowAlbum", "unknowArtist");
    presenter->setMprisPlayer("DeepinMusic", "deepin-music", "Deepin Music Player");
}
