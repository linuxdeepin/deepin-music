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
    return QString(QCryptographicHash::hash(filepath.toLatin1(), QCryptographicHash::Md5).toHex());
}

 QString lengthString(qint64 length)
{
    length = length / 1000;
    QTime t(static_cast<int>(length / 3600), length % 3600 / 60, length % 60);
    return t.toString("mm:ss");
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
    return  QList<QByteArray>();
}

}
