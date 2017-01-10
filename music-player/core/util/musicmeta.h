/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MUSICMETA_H
#define MUSICMETA_H

#include <QFileInfo>

#include "../music.h"

class QFileInfo;
namespace MusicMetaName
{

void pinyinIndex(MusicMeta &info);
QList<QByteArray> detectCodec(const MusicMeta &info);
void updateCodec( MusicMeta &info, QByteArray codecName);
MusicMeta fromLocalFile(const QFileInfo &fileInfo, const QString &hash);
QString hash(const QString &hash);

};

#endif // MUSICMETA_H
