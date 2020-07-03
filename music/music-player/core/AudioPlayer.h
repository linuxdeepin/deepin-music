#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioBuffer>
#include <QMediaContent>
#include <QAudioDecoder>
#include <QAudioOutput>
#include "AudioBufferDevice.h"
#include <QMediaResource>

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    void setSourceFilename(const QString &fileName);

    void play();
    void suspend();
    void reset();

signals:

public slots:

public:

    QAudioFormat *format;
    QAudioDecoder *decoder;

    QAudioOutput *_output;          //音频播放
    AudioBufferDevice *_buffer;     //音频解码
};

#endif // AUDIOPLAYER_H
