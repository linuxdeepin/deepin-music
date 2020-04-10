#ifndef AUDIOBUFFERDEVICE_H
#define AUDIOBUFFERDEVICE_H

#include <QObject>
#include <QIODevice>
#include <QAudioDecoder>
#include <QQueue>
#include <QAudioBuffer>
#include <QBuffer>

class AudioBufferDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioBufferDevice(QAudioDecoder *decoder, QObject *parent = nullptr);
    virtual bool atEnd() const override;
    virtual qint64 bytesAvailable() const override;

protected:
    virtual qint64 readData(char *data, qint64 size) override;
    virtual qint64 writeData(const char *data, qint64 maxSize);

signals:
    void positionChanged(qint64 value);
    void durationChanged(qint64 value);
    void endOfMedia();
    void againMedia();

public slots:
    void sliderReleased(qint64 value);
    void onBufferReady();
    void onFinished();
    void clearQAbuffer();

    void onErroe(QAudioDecoder::Error error);

private:
    QAudioDecoder *_decoder;
    QQueue<QBuffer *> _queue;
    QQueue<QAudioBuffer *> _abuffer_queue;
    bool _isFinished;
    qint64 position = 0;
};

#endif // AUDIOBUFFERDEVICE_H
