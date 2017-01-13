/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
        if (lang == NULL || strlen(lang) == 0) {
            lang = "**";
        }
//        qDebug() <<  name << lang << confidence;
        charsets << name;
    }
//    qDebug() << "match coding list end";


    ucsdet_close(csd);
    return charsets;
}
