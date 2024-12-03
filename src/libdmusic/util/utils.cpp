// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <unicode/ucsdet.h>

#include <QMutex>
#include <QDBusInterface>
#include <QCryptographicHash>
#include <QDebug>
#include <QTextCodec>
#include <QRegExp>

#include <DPinyin>

using namespace std;

static const double pi = 3.1415926535898;
static QMutex dbusMutex;

bool Utils::isChinese(const QChar &c)
{
    return c.unicode() <= 0x9FBF && c.unicode() >= 0x4E00;
}

static inline bool isAlphabeta(const QChar &c)
{
    QRegExp re("[A-Za-z]*");
    return re.exactMatch(c);
}

static inline bool isNumber(const QChar &c)
{
    QRegExp re("[0-9]*");
    return re.exactMatch(c);
}

static inline QString toChinese(const QString &c)
{
    QString pinyin = Dtk::Core::Chinese2Pinyin(c);
    if (pinyin.length() >= 2
            && isNumber(pinyin.at(pinyin.length() - 1))) {
        return pinyin.left(pinyin.length() - 1);
    }
    return pinyin;
}

QStringList Utils::simpleChineseSplit(QString &str)
{
    QStringList wordList;
    bool isLastAlphabeta = false;
    for (auto &c : str) {
        bool isCurAlphabeta = isAlphabeta(c);
        if (isCurAlphabeta) {
            if (!isLastAlphabeta) {
                wordList << c;
            } else {
                wordList.last().append(c);
            }
            continue;
        }
        isLastAlphabeta = isCurAlphabeta;
        //除了中文外，其它字符不作特殊处理
        if (isChinese(c)) {
            wordList << toChinese(c);
            continue;
        } else {
            wordList << c;
            continue;
        }
    }
    return wordList;
}

void Utils::updateChineseMetaInfo(DMusic::MediaMeta &meta)
{
    for (auto &str : simpleChineseSplit(meta.title)) {
        meta.pinyinTitle += str;
        meta.pinyinTitleShort += str.at(0);
    }
    for (auto &str : simpleChineseSplit(meta.album)) {
        meta.pinyinAlbum += str;
        meta.pinyinAlbumShort += str.at(0);
    }
    for (auto &str : simpleChineseSplit(meta.artist)) {
        meta.pinyinArtist += str;
        meta.pinyinArtistShort += str.at(0);
    }
}

QStringList Utils::detectEncodings(const QByteArray &rawData)
{
    QStringList charsets;
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
        ucsdet_close(csd);
        return charsets;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
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

QString Utils::filePathHash(const QString &filepath)
{
    return QString(QCryptographicHash::hash(filepath.toUtf8(), QCryptographicHash::Md5).toHex());
}

void Utils::fft(std::complex<float> *Data, int Log2N, int sign)
{
    int i, j, k, step, length;
    complex<float> wn, temp, deltawn;
    length = 1 << Log2N;
    for (i = 0; i < length; i += 2) {
        temp = Data[i];
        Data[i] = Data[i] + Data[i + 1];
        Data[i + 1] = temp - Data[i + 1];
    }
    for (i = 2; i <= Log2N; i++) {
        wn = 1;
        step = 1 << i;
        deltawn = complex<double>(cos(2.0 * pi / step), sin(sign * 2.0 * pi / step));
        for (j = 0; j < step / 2; j++) {
            for (i = 0; i < length / step; i++) {
                temp = Data[i * step + step / 2 + j] * wn;
                Data[i * step + step / 2 + j] = Data[i * step + j] - temp;
                Data[i * step + j] = Data[i * step + j] + temp;
            }
            wn = wn * deltawn;
        }
    }
    if (sign == 1)
        for (i = 0; i < length; i++)
            Data[i] /= length;
}

QVariant Utils::readDBusProperty(const QString &service, const QString &path, const QString &interface, const char *property, QDBusConnection connection)
{
    // 创建QDBusInterface接口
    dbusMutex.lock();
    QDBusInterface ainterface(service,
                              path,
                              interface,
                              connection);
    if (!ainterface.isValid()) {
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        //cause dead lock if no unlock here,
        dbusMutex.unlock();
        QVariant v(0) ;
        return  v;
    }
    //调用远程的value方法
    QVariant v = ainterface.property(property);
    dbusMutex.unlock();
    return  v;
}

QVariantMap Utils::metaToVariantMap(const DMusic::MediaMeta &meta)
{
    QVariantMap metaMap;
    metaMap.insert("hash", meta.hash);
    metaMap.insert("localPath", meta.localPath);
    metaMap.insert("cuePath", meta.cuePath);
    metaMap.insert("title", meta.title);
    metaMap.insert("artist", meta.artist);
    metaMap.insert("album", meta.album);
    metaMap.insert("lyricPath", meta.lyricPath);

    metaMap.insert("pinyinTitle", meta.pinyinTitle);
    metaMap.insert("pinyinTitleShort", meta.pinyinTitleShort);
    metaMap.insert("pinyinArtist", meta.pinyinArtist);
    metaMap.insert("pinyinArtistShort", meta.pinyinArtistShort);
    metaMap.insert("pinyinAlbum", meta.pinyinAlbum);
    metaMap.insert("pinyinAlbumShort", meta.pinyinAlbumShort);

    metaMap.insert("filetype", meta.filetype);
    metaMap.insert("mmType", meta.mmType);
    metaMap.insert("timestamp", meta.timestamp);
    metaMap.insert("offset", meta.offset);
    metaMap.insert("length", meta.length);
    metaMap.insert("size", meta.size);
    metaMap.insert("track", meta.track);
    metaMap.insert("editor", meta.editor);
    metaMap.insert("composer", meta.composer);
    metaMap.insert("creator", meta.creator);
    metaMap.insert("searchID", meta.searchID);
    metaMap.insert("coverUrl", meta.coverUrl);

    metaMap.insert("hasimage", meta.hasimage);
    metaMap.insert("favourite", meta.favourite);
    metaMap.insert("invalid", meta.invalid);
    metaMap.insert("loadCover", meta.loadCover);
    metaMap.insert("toDelete", meta.toDelete);
    metaMap.insert("codec", meta.codec);

    // qml runtime properties
    metaMap.insert("inMulitSelect", meta.inMuiltSelect);
    metaMap.insert("dragFlag", meta.dragFlag);

    return metaMap;
}

QVariantMap Utils::albumToVariantMap(const DMusic::AlbumInfo &album)
{
    QVariantMap infoMap;
    infoMap.insert("name", album.name);
    infoMap.insert("pinyin", album.pinyin);
    infoMap.insert("artist", album.artist);
    infoMap.insert("timestamp", album.timestamp);

    QVariantMap allMetaMap;
    QMapIterator<QString, DMusic::MediaMeta> i(album.musicinfos);
    while (i.hasNext()) {
        i.next();
        allMetaMap.insert(i.key(), Utils::metaToVariantMap(i.value()));
    }

    infoMap.insert("musicinfos", allMetaMap);
    return infoMap;
}

QVariantMap Utils::artistToVariantMap(const DMusic::ArtistInfo &artist)
{
    QVariantMap infoMap;
    infoMap.insert("name", artist.name);
    infoMap.insert("pinyin", artist.pinyin);
    infoMap.insert("timestamp", artist.timestamp);

    QVariantMap allMetaMap;
    QMapIterator<QString, DMusic::MediaMeta> i(artist.musicinfos);
    while (i.hasNext()) {
        i.next();
        allMetaMap.insert(i.key(), Utils::metaToVariantMap(i.value()));
    }

    infoMap.insert("musicinfos", allMetaMap);
    return infoMap;
}

int Utils::simplifyPlaylistSortType(const int &sortType)
{
    int  curSortType = DmGlobal::SortByAddTime;
    switch (sortType) {
    case DmGlobal::SortByAddTimeASC:
    case DmGlobal::SortByAddTimeDES: {
        curSortType = DmGlobal::SortByAddTime;
        break;
    }
    case DmGlobal::SortByTitleASC:
    case DmGlobal::SortByTitleDES: {
        curSortType = DmGlobal::SortByTitle;
        break;
    }
    case DmGlobal::SortByAblumASC:
    case DmGlobal::SortByAblumDES: {
        curSortType = DmGlobal::SortByAblum;
        break;
    }
    case DmGlobal::SortByArtistASC:
    case DmGlobal::SortByArtistDES: {
        curSortType = DmGlobal::SortByArtist;
        break;
    }
    case DmGlobal::SortByCustomASC:
    case DmGlobal::SortByCustomDES: {
        curSortType = DmGlobal::SortByCustom;
        break;
    }
    default:
        curSortType = DmGlobal::SortByAddTime;
        break;
    }
    return curSortType;
}

QVariantMap Utils::playlistToVariantMap(const DMusic::PlaylistInfo &playlist)
{
    QVariantMap metaMap;

    metaMap.insert("uuid", playlist.uuid);
    metaMap.insert("displayName", playlist.displayName);
    metaMap.insert("icon", playlist.icon);
    metaMap.insert("sortMetas", playlist.sortMetas);
    metaMap.insert("sortType", simplifyPlaylistSortType(playlist.sortType));
    metaMap.insert("orderType", playlist.orderType);
    metaMap.insert("sortID", playlist.sortID);
    metaMap.insert("editmode", playlist.editmode);
    metaMap.insert("readonly", playlist.readonly);
    metaMap.insert("hide", playlist.hide);
    metaMap.insert("active", playlist.active);
    metaMap.insert("playStatus", playlist.playStatus);

    return metaMap;
}

bool Utils::containsStr(QString searchText, QString text)
{
    text = QString(text).remove("\r").remove("\n");
    bool chineseFlag = false;
    for (auto ch : searchText) {
        if (isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }
    if (chineseFlag) {
        return text.contains(searchText);
    } else {
        auto curTextList = simpleChineseSplit(text);
        QString curTextListStr = "";
        if (!curTextList.isEmpty()) {
            for (auto mText : curTextList) {
                if (mText.contains(searchText, Qt::CaseInsensitive)) {
                    return true;
                }
                curTextListStr += mText;
            }
//            curTextListStr = QString(curTextListStr.remove(" "));
            if (curTextListStr.contains(searchText, Qt::CaseInsensitive)) {
                return true;
            }
        }
        return text.contains(searchText, Qt::CaseInsensitive);
    }
}
