#pragma once

#include <QScopedPointer>
#include <QWidget>

class WaterProgressInter;
class WaterProgressBorder;
class WaterProgress : public QWidget
{
    Q_OBJECT
public:
    explicit WaterProgress(QWidget *parent = 0);

    void updateSize(QSize size);

private:
    WaterProgressInter *water;
    WaterProgressBorder *waterBoder;
};

class WaterProgressInterPrivate;
class WaterProgressInter : public QWidget
{
    Q_OBJECT
public:
    explicit WaterProgressInter(QWidget *parent = 0);
    ~WaterProgressInter();

public slots:
    void start();
    void stop();
    void setProgress(int);

protected:
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QScopedPointer<WaterProgressInterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), WaterProgressInter)
};

class WaterProgressBorder : public QWidget
{
    Q_OBJECT
public:
    explicit WaterProgressBorder(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
};

