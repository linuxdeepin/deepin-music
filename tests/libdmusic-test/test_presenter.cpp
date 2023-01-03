// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>

#include <unistd.h>
#include <gtest/gtest.h>

#include "presenter.h"

TEST(Presenter, Presenter)
{
    Presenter * presenter = new Presenter("unknowAlbum", "unknowArtist");
    presenter->setMprisPlayer("DeepinMusic", "deepin-music", "Deepin Music Player");
}
