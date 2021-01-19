#include "musicstackedwidget.h"

#include <QPropertyAnimation>

#include "ac-desktop-define.h"

MusicStatckedWidget::MusicStatckedWidget(QWidget *parent)
    : DStackedWidget(parent)
{
    setObjectName("MainWidgetStatcked");
}

void MusicStatckedWidget::animationToUp()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(AnimationDelay);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(0, -height()));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    // 防止挡住歌词控件
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicStatckedWidget::hide);

//    this->resize(size);
    this->show();

    animation->start();
}

void MusicStatckedWidget::animationToDown()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setDuration(AnimationDelay);
    animation->setStartValue(QPoint(0, -height()));
    animation->setEndValue(QPoint(0, 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    this->show();

    animation->start();
}

void MusicStatckedWidget::animationImportToDown(const QSize &size)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, -size.height()));
    animation->setEndValue(QPoint(0, 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);

    this->resize(size);
    this->show();
    animation->start();
}

void MusicStatckedWidget::animationImportToLeft(const QSize &size)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(QPoint(0, 50));
    animation->setEndValue(QPoint(-size.width(), 50));

    animation->connect(animation, &QPropertyAnimation::finished,
                       animation, &QPropertyAnimation::deleteLater);
    animation->connect(animation, &QPropertyAnimation::finished,
                       this, &MusicStatckedWidget::hide);
    this->resize(size);

    animation->start();
}
