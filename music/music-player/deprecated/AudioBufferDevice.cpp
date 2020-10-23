#include "AudioBufferDevice.h"
#include <QByteArray>

AudioBufferDevice::AudioBufferDevice(QAudioDecoder *decoder, QObject *parent):
    QIODevice(parent),
    _decoder(decoder),
    _isFinished(false)
{
    connect(_decoder, SIGNAL(bufferReady()), this, SLOT(onBufferReady()));
    connect(_decoder, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(_decoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(onErroe(QAudioDecoder::Error)));
}

bool AudioBufferDevice::atEnd() const
{
    return _isFinished && _queue.empty();
}

qint64 AudioBufferDevice::readData(char *data, qint64 size)
{
    if (position == _abuffer_queue.size() && position == 0 && _abuffer_queue.size() == 0) {
        Q_EMIT againMedia();
        return 0;
    }

    if (position == _abuffer_queue.size()) {
        //qDebug() << "数据已播放完成";
        Q_EMIT endOfMedia();
        return  0;
    }

    if (_queue.empty() && _abuffer_queue.empty()) {
        return 0;
    }

    if (_queue.empty()) {
        //QAudioBuffer *buffer = _abuffer_queue.first();
        QAudioBuffer *buffer = _abuffer_queue.at(static_cast<int>(position));

        qint32 *data = static_cast<qint32 *>(buffer->data());
        int len = buffer->sampleCount();
        QByteArray retpcm;

        for (int i = 0; i < len; i += 1) {
            qint32 pcm = data[i];
            retpcm.append((char *)&pcm, sizeof(pcm));
        }

        auto qb = new QBuffer();
        qb->setData(retpcm);

        _queue.push_back(qb);
        //_abuffer_queue.removeFirst();
        //delete buffer;
    }

    QBuffer *buffer = _queue.first();
    if (!buffer->isOpen()) {
        buffer->open(QIODevice::ReadOnly);
    }

    //qDebug() << "音乐播放中....." << _abuffer_queue.size();
    qint64 n = buffer->read(data, size);
    Q_EMIT positionChanged(position++);

    if (buffer->atEnd()) {
        _queue.removeFirst();
        delete buffer;
    }

    return n;
}

qint64 AudioBufferDevice::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return 0;
}

qint64 AudioBufferDevice::bytesAvailable() const
{
    if (_queue.empty()) {
        return 0;
    } else {
        return _queue.first()->bytesAvailable();
    }
}

void AudioBufferDevice::sliderReleased(qint64 value)
{
    position = value;
    //qDebug() << value << "-" << _abuffer_queue.size();
}

void AudioBufferDevice::onBufferReady()
{
    QAudioBuffer buffer = _decoder->read();
    _abuffer_queue.push_back(new QAudioBuffer(buffer));
    Q_EMIT durationChanged(1);
}

void AudioBufferDevice::onFinished()
{
    _isFinished = true;
    //Q_EMIT durationChanged(_abuffer_queue.size());
}

void AudioBufferDevice::onErroe(QAudioDecoder::Error error)
{
    qDebug() << error;
}

void AudioBufferDevice::clearQAbuffer()
{
    position = 0;
    _queue.clear();
    _abuffer_queue.clear();
}
