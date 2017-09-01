#include "buttongroup.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

class ButtonGroupPrivate
{
public:
    ButtonGroupPrivate(ButtonGroup *parent) : q_ptr(parent){}

    QHBoxLayout *layout = nullptr;
    QButtonGroup *group  = nullptr;
    ButtonGroup *q_ptr;
    Q_DECLARE_PUBLIC(ButtonGroup)
};

ButtonGroup::ButtonGroup(QWidget *parent) :
    QFrame(parent), d_ptr(new ButtonGroupPrivate(this))
{
    Q_D(ButtonGroup);
    setObjectName("ButtonGroup");
    d->group = new QButtonGroup;
    d->layout = new QHBoxLayout(this);
    d->layout->setSpacing(0);
    d->layout->setMargin(0);

    connect(d->group,static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonReleased),
            this, [=](int){
        Q_EMIT buttonChecked(d->group->checkedId());
    });
}

ButtonGroup::~ButtonGroup()
{

}

void ButtonGroup::setCheckedButton(int id)
{
    Q_D(ButtonGroup);
    if (d->group->button(id)) {
        d->group->button(id)->setChecked(true);
    }
}

void ButtonGroup::setButtons(const QStringList &texts)
{
    Q_D(ButtonGroup);
    int i = 0;
    for (auto text: texts) {
        auto bt = new QPushButton(text);
        bt->setFixedWidth(36);
        bt->setFixedHeight(22);
        bt->setCheckable(true);

        if (texts.length() <= 1)
            bt->setObjectName("ButtonGroupSingle");
//            bt->setStyleSheet("QPushButton{border: 1px solid red; border-radius: 4.0px}");
        else {
            if (i == 0)
                bt->setObjectName("ButtonGroupBegin");
//                bt->setStyleSheet("QPushButton{"
//                                  "border: 1px solid red;"
//                                  "border-top-left-radius: 4.0px;"
//                                  "border-bottom-left-radius: 4.0px;"
//                                  "}");
            else if (i == texts.length() -1) {
                    bt->setObjectName("ButtonGroupEnd");
//                bt->setStyleSheet("QPushButton{"
//                                  "border: 1px solid red;"
//                                  "border-top-right-radius: 4.0px;"
//                                  "border-bottom-right-radius: 4.0px;"
//                                  "}");
            } else {

                    bt->setObjectName("ButtonGroupMiddle");
//                bt->setStyleSheet("QPushButton{"
//                                  "border-top: 1px solid red;"
//                                  "border-bottom: 1px solid red;"
//                                  "}");
            }
        }
        d->group->addButton(bt, i);
        d->layout->addWidget(bt);
        i++;
    }
    d->layout->addStretch();
}
