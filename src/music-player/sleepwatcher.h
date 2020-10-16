#ifndef SLEEPWATCHER_H
#define SLEEPWATCHER_H

#include <QThread>

class sleepwatcher : public QThread
{
    Q_OBJECT
public:
    explicit sleepwatcher(const QString &path,QObject *parent = nullptr);
    void run() override;
signals:
    void filechanged(bool changed); //文件改变
public slots:
private:
    QStringList wpath;
};

#endif // SLEEPWATCHER_H
