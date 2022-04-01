/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     Zhang Wenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "playerbase.h"
#include <QDebug>

int PlayerBase::INT_LAST_PROGRESS_FLAG = 1; //1标识当前歌曲有上一次的播放进度，0标识当前歌曲没有上一次的播放记录
PlayerBase::PlayerBase(QObject *parent)
    : QObject(parent)
{

}

PlayerBase::~PlayerBase()
{

}

void PlayerBase::initCddaTrack()
{
    qDebug() << __func__;
}

QList<MediaMeta> PlayerBase::getCdaMetaInfo()
{
    qDebug() << __func__;
    return QList<MediaMeta>();
}

void PlayerBase::setEqualizerEnabled(bool enabled)
{
    qDebug() << __func__;
    Q_UNUSED(enabled);
}

void PlayerBase::loadFromPreset(uint index)
{
    qDebug() << __func__;
    Q_UNUSED(index);
}

void PlayerBase::setPreamplification(float value)
{
    qDebug() << __func__;
    Q_UNUSED(value);
}

void PlayerBase::setAmplificationForBandAt(float amp, uint bandIndex)
{
    qDebug() << __func__;
    Q_UNUSED(amp);
    Q_UNUSED(bandIndex);
}

float PlayerBase::amplificationForBandAt(uint bandIndex)
{
    qDebug() << __func__;
    Q_UNUSED(bandIndex);
    return 1.0;
}

float PlayerBase::preamplification()
{
    qDebug() << __func__;
    return 1.0;
}
