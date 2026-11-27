// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APESEEKFILE_H
#define APESEEKFILE_H

#include <QString>

class QFileInfo;

// Decode a local APE file into a seekable WAV, on a worker thread.
bool writeApeSeekFile(const QFileInfo &source, const QString &destination);
bool isApeSeekFileReady(const QString &path);

#endif // APESEEKFILE_H
