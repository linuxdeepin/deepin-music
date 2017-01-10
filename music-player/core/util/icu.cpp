#include "icu.h"

#include <QDebug>
#include <unicode/ucsdet.h>
#include <QTextCodec>

namespace ICU
{

QList<QByteArray> codeName(const QByteArray &cueByte)
{
    QList<QByteArray> charsets;
    QByteArray charset = QTextCodec::codecForLocale()->name();

    charsets << charset;

    const char *data = cueByte.data();
    int32_t len = cueByte.size();

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
        int32_t confidence = ucsdet_getConfidence(csm[match], &status);

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


}
