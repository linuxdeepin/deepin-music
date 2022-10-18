#ifndef CHECKDATAZEROTHREAD_H
#define CHECKDATAZEROTHREAD_H

#include <QThread>

class CheckDataZeroThread : public QThread
{
    Q_OBJECT
public:
    explicit CheckDataZeroThread(QObject *parent = nullptr);

    void quitThread() {m_bExit = true;}

    void initTimeParams();

signals:
    void sigPlayNextSong();
    // 发送剩余进度
    void sigExtraTime(qint64 time);

protected:
    void run() override;

    void resetParam();

private:
    bool m_bExit = false;
    qint64 m_currentTime = 0; //当前进度的时间
    qint64 m_step = 0;        //步进
    qint64 m_duration = 0;    //时间总长度
};

#endif // CHECKDATAZEROTHREAD_H
