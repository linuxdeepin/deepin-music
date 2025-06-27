// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "util/log.h"

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
    bool result = c.unicode() <= 0x9FBF && c.unicode() >= 0x4E00;
    qCDebug(dmMusic) << "Checking if character is Chinese:" << c << "Result:" << result;
    return result;
}

static inline bool isAlphabeta(const QChar &c)
{
    qCDebug(dmMusic) << "Checking if character is alphabeta:" << c;
    QRegExp re("[A-Za-z]*");
    return re.exactMatch(c);
}

static inline bool isNumber(const QChar &c)
{
    qCDebug(dmMusic) << "Checking if character is number:" << c;
    QRegExp re("[0-9]*");
    return re.exactMatch(c);
}

static inline QString toChinese(const QString &c)
{
    qCDebug(dmMusic) << "Converting to Chinese:" << c;
    QString pinyin = Dtk::Core::Chinese2Pinyin(c);
    if (pinyin.length() >= 2
            && isNumber(pinyin.at(pinyin.length() - 1))) {
        return pinyin.left(pinyin.length() - 1);
    }
    return pinyin;
}

QStringList Utils::simpleChineseSplit(QString &str)
{
    qCDebug(dmMusic) << "Splitting Chinese string:" << str;
    QStringList wordList;
    bool isLastAlphabeta = false;
    for (auto &c : str) {
        bool isCurAlphabeta = isAlphabeta(c);
        if (isCurAlphabeta) {
            qCDebug(dmMusic) << "Current character is alphabeta:" << c;
            if (!isLastAlphabeta) {
                wordList << c;
                qCDebug(dmMusic) << "Add alphabeta to word list:" << c;
            } else {
                wordList.last().append(c);
                qCDebug(dmMusic) << "Add alphabeta to last word list:" << c;
            }
            continue;
        }
        isLastAlphabeta = isCurAlphabeta;
        //除了中文外，其它字符不作特殊处理
        if (isChinese(c)) {
            wordList << toChinese(c);
            qCDebug(dmMusic) << "Add Chinese to word list:" << toChinese(c);
            continue;
        } else {
            wordList << c;
            qCDebug(dmMusic) << "Add non-Chinese to word list:" << c;
            continue;
        }
    }
    qCDebug(dmMusic) << "Splitted Chinese string:" << wordList;
    return wordList;
}

void Utils::updateChineseMetaInfo(DMusic::MediaMeta &meta)
{
    qCDebug(dmMusic) << "Updating Chinese meta info for:" << meta.title;
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
    qCDebug(dmMusic) << "Updated Chinese meta info end.";
}

QStringList Utils::detectEncodings(const QByteArray &rawData)
{
    qCDebug(dmMusic) << "Detecting encodings for raw data size:" << rawData.size();
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
        qCWarning(dmMusic) << "Failed to open charset detector, error:" << status;
        return charsets;
    }

    ucsdet_setText(csd, data, len, &status);
    if (status != U_ZERO_ERROR) {
        qCWarning(dmMusic) << "Failed to set text for charset detection, error:" << status;
        ucsdet_close(csd);
        return charsets;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        qCWarning(dmMusic) << "Failed to detect charsets, error:" << status;
        ucsdet_close(csd);
        return charsets;
    }

    if (matchCount > 0) {
        charsets.clear();
        qCDebug(dmMusic) << "Found" << matchCount << "charset matches";
    }

    for (int32_t match = 0; match < matchCount; match += 1) {
        const char *name = ucsdet_getName(csm[match], &status);
        const char *lang = ucsdet_getLanguage(csm[match], &status);
        if (lang == nullptr || strlen(lang) == 0) {
            lang = "**";
        }
        charsets << name;
    }

    ucsdet_close(csd);
    qCDebug(dmMusic) << "Detected encodings:" << charsets;
    return charsets;
}

QString Utils::filePathHash(const QString &filepath)
{
    qCDebug(dmMusic) << "Generating file path hash for:" << filepath;
    QString hash = QString(QCryptographicHash::hash(filepath.toUtf8(), QCryptographicHash::Md5).toHex());
    qCDebug(dmMusic) << "Generated hash:" << hash;
    return hash;
}

void Utils::fft(std::complex<float> *Data, int Log2N, int sign)
{
    qCDebug(dmMusic) << "Performing FFT - Log2N:" << Log2N << "Sign:" << sign;
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
    if (sign == 1) {
        qCDebug(dmMusic) << "Performing IFFT - Sign:" << sign;
        for (i = 0; i < length; i++)
            Data[i] /= length;
    }
    qCDebug(dmMusic) << "FFT finished.";
}

QVariant Utils::readDBusProperty(const QString &service, const QString &path, const QString &interface, const char *property, QDBusConnection connection)
{
    // qCDebug(dmMusic) << "Reading DBus property -" << "Service:" << service << "Path:" << path << "Interface:" << interface << "Property:" << property;
    
    dbusMutex.lock();
    QDBusInterface ainterface(service,
                              path,
                              interface,
                              connection);
    if (!ainterface.isValid()) {
        qCWarning(dmMusic) << "Failed to create DBus interface:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        dbusMutex.unlock();
        QVariant v(0);
        return v;
    }
    //调用远程的value方法
    QVariant v = ainterface.property(property);
    dbusMutex.unlock();
    
    if (!v.isValid()) {
        qCWarning(dmMusic) << "Failed to read DBus property:" << property;
    }
    
    return v;
}

QVariantMap Utils::metaToVariantMap(const DMusic::MediaMeta &meta)
{
    qCDebug(dmMusic) << "Converting meta to variant map:" << meta.title;
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
    qCDebug(dmMusic) << "Converting album to variant map:" << album.name;
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
    qCDebug(dmMusic) << "Converting artist to variant map:" << artist.name;
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
    qCDebug(dmMusic) << "Simplifying playlist sort type:" << sortType;
    int  curSortType = DmGlobal::SortByAddTime;
    switch (sortType) {
    case DmGlobal::SortByAddTimeASC:
    case DmGlobal::SortByAddTimeDES: {
        qCDebug(dmMusic) << "Playlist sort type is add time.";

        curSortType = DmGlobal::SortByAddTime;
        break;
    }
    case DmGlobal::SortByTitleASC:
    case DmGlobal::SortByTitleDES: {
        qCDebug(dmMusic) << "Playlist sort type is title.";
        curSortType = DmGlobal::SortByTitle;
        break;
    }
    case DmGlobal::SortByAblumASC:
    case DmGlobal::SortByAblumDES: {
        qCDebug(dmMusic) << "Playlist sort type is album.";
        curSortType = DmGlobal::SortByAblum;
        break;
    }
    case DmGlobal::SortByArtistASC:
    case DmGlobal::SortByArtistDES: {
        qCDebug(dmMusic) << "Playlist sort type is artist.";
        curSortType = DmGlobal::SortByArtist;
        break;
    }
    case DmGlobal::SortByCustomASC:
    case DmGlobal::SortByCustomDES: {
        qCDebug(dmMusic) << "Playlist sort type is custom.";
        curSortType = DmGlobal::SortByCustom;
        break;
    }
    default:
        qCDebug(dmMusic) << "Playlist sort type is unknown.";
        curSortType = DmGlobal::SortByAddTime;
        break;
    }
    return curSortType;
}

QVariantMap Utils::playlistToVariantMap(const DMusic::PlaylistInfo &playlist)
{
    qCDebug(dmMusic) << "Converting playlist to variant map:" << playlist.displayName;
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
    qCDebug(dmMusic) << "Performing string search - Search text:" << searchText;
    text = QString(text).remove("\r").remove("\n");
    bool chineseFlag = false;
    for (auto ch : searchText) {
        if (isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }
    
    if (chineseFlag) {
        qCDebug(dmMusic) << "Performing Chinese text search";
        return text.contains(searchText);
    } else {
        auto curTextList = simpleChineseSplit(text);
        QString curTextListStr = "";
        if (!curTextList.isEmpty()) {
            for (auto mText : curTextList) {
                if (mText.contains(searchText, Qt::CaseInsensitive)) {
                    qCDebug(dmMusic) << "return true, Chinese text search - Matched text:" << mText;
                    return true;
                }
                curTextListStr += mText;
            }
            if (curTextListStr.contains(searchText, Qt::CaseInsensitive)) {
                qCDebug(dmMusic) << "return true, English text search - Matched text:" << curTextListStr;
                return true;
            }
        }
        return text.contains(searchText, Qt::CaseInsensitive);
    }
}
