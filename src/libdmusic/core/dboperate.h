/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DBOPERATE_H
#define DBOPERATE_H

#include "global.h"

class DBOperate : public QObject
{
    Q_OBJECT
public:
    DBOperate(QStringList supportedSuffixs, QObject *parent = nullptr);

public slots:
    void slotImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlay,
                         const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                         const QString &playlistHash = "", const bool &playFalg = false);

signals:
    void signalAddOneMeta(QStringList playlistHashs, DMusic::MediaMeta meta);
    void signalImportFinished(QStringList playlistHashs, int failCount, int sucessCount, int existCount, QString mediaHash);

private:
    QStringList          m_supportedSuffixs;
};

#endif //DBOPERATE_H
