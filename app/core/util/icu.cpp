#include "icu.h"

#include <unicode/ucsdet.h>
#include <QDebug>
#include <QTextCodec>
namespace ICU
{

QByteArray codeName(const QByteArray &cueByte)
{
    QByteArray charset = QTextCodec::codecForLocale()->name();

    const char *data = cueByte.data();
    int32_t len = cueByte.size();

    UCharsetDetector *csd;
    const UCharsetMatch **csm;
    int32_t matchCount = 0;

    UErrorCode status = U_ZERO_ERROR;

    csd = ucsdet_open(&status);
    if (status != U_ZERO_ERROR) {
        return charset;
    }

    ucsdet_setText(csd, data, len, &status);
    if (status != U_ZERO_ERROR) {
        return charset;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        return charset;
    }

//    qDebug() << "match coding list" << charset;
//    for (int32_t match = 0; match < matchCount; match += 1) {
//        const char *name = ucsdet_getName(csm[match], &status);
//        const char *lang = ucsdet_getLanguage(csm[match], &status);
//        int32_t confidence = ucsdet_getConfidence(csm[match], &status);

//        if (lang == NULL || strlen(lang) == 0) {
//            lang = "**";
//        }

//        qDebug() <<  name << lang << confidence;
//    }
//    qDebug() << "match coding list end";

    if (matchCount > 0) {
        auto charsetName = strdup(ucsdet_getName(csm[0], &status));
        if (status != U_ZERO_ERROR) {
            return charset;
        }
        return charsetName;
    }

    ucsdet_close(csd);
    return charset;
}


}
