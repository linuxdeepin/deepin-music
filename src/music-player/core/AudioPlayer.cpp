#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(QObject *parent)
{
    Q_UNUSED(parent)
    format = new QAudioFormat();
    format->setSampleRate(44100);
    format->setChannelCount(2);
    format->setCodec("audio/pcm");
    format->setSampleSize(32);
    format->setByteOrder(QAudioFormat::LittleEndian);
    format->setSampleType(QAudioFormat::SignedInt);

    decoder = new QAudioDecoder(this);
    decoder->setAudioFormat(*format);

    qDebug() << "isvalide: " << format->isValid();

    _output = new QAudioOutput(*format, this);
    _buffer = new AudioBufferDevice(decoder, this);
    _buffer->open(QIODevice::ReadOnly);
}

void AudioPlayer::play()
{
    _output->start(_buffer);
}

void AudioPlayer::suspend()
{
    // 暂停
    _output->suspend();
}

void AudioPlayer::reset()
{
    // 停止
    _output->reset();
}

void AudioPlayer::setSourceFilename(const QString &fileName)
{
    //qDebug() << "setSourceFilename" << fileName ;

    _buffer->clearQAbuffer();
    decoder->stop();
    decoder->setSourceFilename(fileName);
    decoder->start();
}
