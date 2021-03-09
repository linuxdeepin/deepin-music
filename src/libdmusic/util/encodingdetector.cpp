/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
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

#include "encodingdetector.h"

#include <QDebug>
#include <QTextCodec>

#include <unicode/ucsdet.h>

using namespace DMusic;

QList<QByteArray> EncodingDetector::detectEncodings(const QByteArray &rawData)
{
    QList<QByteArray> charsets;
    QByteArray charset = QTextCodec::codecForLocale()->name();
    charsets << charset;

    const char *data = rawData.data();
    int32_t len = rawData.size();

    UCharsetDetector *csd;
    const UCharsetMatch **csm;
    int32_t matchCount = 0;

    UErrorCode status = U_ZERO_ERROR;

    csd = ucsdet_open(&status);
    if (status != U_ZERO_ERROR) {
        return charsets;
    }

    ucsdet_setText(csd, data, len, &status);
    if (status != U_ZERO_ERROR) {
        return charsets;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        return charsets;
    }

    if (matchCount > 0) {
        charsets.clear();
    }

//    qDebug() << "match coding list" << charset;
    for (int32_t match = 0; match < matchCount; match += 1) {
        const char *name = ucsdet_getName(csm[match], &status);
        const char *lang = ucsdet_getLanguage(csm[match], &status);
//        int32_t confidence = ucsdet_getConfidence(csm[match], &status);
        if (lang == nullptr || strlen(lang) == 0) {
            lang = "**";
        }
//        qDebug() <<  name << lang << confidence;
        charsets << name;
    }
//    qDebug() << "match coding list end";


    ucsdet_close(csd);
    return charsets;
}
