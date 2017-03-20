#include "mediameta.h"

#include <QTime>
#include <QFileInfo>
#include <QCryptographicHash>

#include "util/pinyinsearch.h"
#include "metadetector.h"

using namespace DMusic;

void MediaMeta::updateSearchIndex()
{
    for (auto &str : PinyinSearch::simpleChineseSplit(this->title)) {
        this->pinyinTitle += str;
        this->pinyinTitleShort += str.at(0);
    }
    for (auto &str : PinyinSearch::simpleChineseSplit(this->artist)) {
        this->pinyinArtist += str;
        this->pinyinArtistShort += str.at(0);
    }
    for (auto &str : PinyinSearch::simpleChineseSplit(this->album)) {
        this->pinyinAlbum += str;
        this->pinyinAlbumShort += str.at(0);
    }
}

void MediaMeta::updateCodec(const QByteArray &codec)
{
    QFileInfo cueFi(this->cuePath);
    if (cueFi.exists()) {
        MetaDetector::updateCueFileTagCodec(this, cueFi, codec);
    } else {
        MetaDetector::updateMediaFileTagCodec(this, codec, true);
    }
}

MediaMeta MediaMeta::fromLocalFile(const QFileInfo &fileInfo)
{
    MediaMeta meta;
    meta.hash = filepathHash(fileInfo.absoluteFilePath());
    MetaDetector::updateMetaFromLocalfile((&meta), fileInfo);
    return  meta;
}

namespace DMusic
{
QString filepathHash(const QString &filepath)
{
    return QString(QCryptographicHash::hash(filepath.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString lengthString(qint64 length)
{
    length = length / 1000;
    int hour = static_cast<int>(length / 3600);
    QTime t(0, length % 3600 / 60, length % 60);
    if (hour > 0) {
        return QString("%1:%2").arg(hour).arg(t.toString("mm:ss"));
    } else {
        return t.toString("mm:ss");
    }
}

QString sizeString(qint64 sizeByte)
{
    QString text;
    if (sizeByte < 1024) {
        text.sprintf("%.1fB", sizeByte / 1.0);
        return text;
    }
    if (sizeByte < 1024 * 1024) {
        text.sprintf("%.1fK", sizeByte / 1024.0);
        return text;
    }
    if (sizeByte < 1024 * 1024 * 1024) {
        text.sprintf("%.1fM", sizeByte / 1024.0 / 1024.0);
        return text;
    }
    text.sprintf("%.1fG", sizeByte / 1024.0 / 1024.0 / 1024.0);
    return text;
}

QList<QByteArray> detectMetaEncodings(MetaPtr meta)
{
    return  MetaDetector::detectEncodings(meta);
}

}
