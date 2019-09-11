#ifndef MusicLyric_H
#define MusicLyric_H

#include <QVector>

class MusicLyric
{
    public:
        MusicLyric();
        void getFromFile(QString dir);
        QString getLineAt(int index);
        int getCount();
        int getIndex(qint64 pos);
        qint64 getPostion(int index);

    private:
        QString filedir;
        double offset;
        QVector<qint64> postion;
        QVector<QString> line;
};

#endif // LYRIC_H
