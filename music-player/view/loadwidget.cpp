#include "loadwidget.h"

#include <QVBoxLayout>
#include <QLabel>

#include "widget/waterprogress.h"
#include "view/helper/thememanager.h"

class LoadWidgetPrivate
{
public:
    LoadWidgetPrivate(LoadWidget *parent) : q_ptr(parent) {}

    LoadWidget *q_ptr;
    Q_DECLARE_PUBLIC(LoadWidget)
};

LoadWidget::LoadWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new LoadWidgetPrivate(this))
{
//    setObjectName("LoadWidget");
    ThemeManager::instance()->regisetrWidget(this);

    auto water = new WaterProgress(this);

    auto label = new QLabel(tr("Loading..."));
    label->setObjectName("LoadWidgetLabel");

    auto vbox = new QVBoxLayout;
    setLayout(vbox);

    vbox->addStretch();
    vbox->addWidget(water, 0, Qt::AlignCenter);
    vbox->addSpacing(10);
    vbox->addWidget(label, 0, Qt::AlignCenter);
    vbox->addStretch();
}

LoadWidget::~LoadWidget()
{

}
