#include "scheduledemowidget.h"
#include <QVBoxLayout>
#include <QDebug>
#include <DLabel>
#include <DPushButton>


ScheduleDemoWidget::ScheduleDemoWidget(DWidget *parent): DWidget(parent)
{

}

void ScheduleDemoWidget::slotReceivce(QVariant data, Reply *reply)
{
}

void ScheduleDemoWidget::process(const QString &semantic)
{
    //助手中显示的界面
    QVBoxLayout *boxLayout = new QVBoxLayout;
    boxLayout->setSpacing(10);
    boxLayout->setMargin(10);
    boxLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->setLayout(boxLayout);

    for (int i = 0; i < 5; i++) {
        DLabel *label1 = new DLabel("日程" + QString::number(i), this);
        boxLayout->addWidget(label1);
    }

    DPushButton *btn = new DPushButton("发送消息给助手显示", this);
    boxLayout->addWidget(btn);
    connect(btn, &DPushButton::clicked, this, [&]() {
        qDebug() << __FUNCTION__ << "clicked";
        emit signaleSendMessage("这是查询到的日程");
    });

}
