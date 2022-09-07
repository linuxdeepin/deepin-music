// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "playerbase.h"
#include <QDebug>

int PlayerBase::INT_LAST_PROGRESS_FLAG = 1; //1标识当前歌曲有上一次的播放进度，0标识当前歌曲没有上一次的播放记录
PlayerBase::PlayerBase(QObject *parent)
    : QObject(parent)
{

}
