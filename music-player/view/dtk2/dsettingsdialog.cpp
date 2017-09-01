#include "dsettingsdialog.h"

#include <QDebug>
#include <QHBoxLayout>

#include <DSettingsOption>
#include <DSettings>

#include "settings/content.h"
#include "settings/navigation.h"
#include "settings/widgetfactory.h"

#include "dthememanager.h"
#include "dwindowclosebutton.h"
#include "dspinbox.h"
#include "dbuttonlist.h"
#include "doptionlist.h"

DWIDGET_USE_NAMESPACE

namespace dm { //DMUSIC_BEGIN_NAMESPACE

class DSettingsDialogPrivate
{
public:
    DSettingsDialogPrivate(DSettingsDialog *parent) : q_ptr(parent) {}

    Navigation      *leftFrame;

    QFrame          *rightFrame;
    QVBoxLayout     *rightLayout;
    Content         *content;

    DSettingsDialog *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsDialog)
};


DSettingsDialog::DSettingsDialog(QWidget *parent) :
    DAbstractDialog(parent), d_ptr(new DSettingsDialogPrivate(this))
{
    Q_D(DSettingsDialog);

    D_THEME_INIT_WIDGET(dialogs/DSettingsDialog);

    setObjectName("DSettingsDialog");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->leftFrame = new Navigation;
    d->leftFrame->layout()->setContentsMargins(0, 30, 0, 0);

    layout->addWidget(d->leftFrame);

    d->rightFrame = new QFrame;
    d->rightFrame->setObjectName("RightFrame");
    d->rightLayout = new QVBoxLayout(d->rightFrame);
    d->rightLayout->setContentsMargins(1, 5, 5, 0);
    d->rightLayout->setSpacing(0);

    d->content = new Content;

    auto closeBt = new DWindowCloseButton;
    d->rightLayout->addWidget(closeBt, 0, Qt::AlignVCenter | Qt::AlignRight);
    d->rightLayout->addWidget(d->content);

    layout->addWidget(d->rightFrame);
    setFixedWidth(680);

    connect(d->leftFrame, &Navigation::selectedGroup, d->content, &Content::onScrollToGroup);
    connect(d->content, &Content::scrollToGroup, d->leftFrame, &Navigation::onSelectGroup);
    connect(closeBt, &DWindowCloseButton::clicked, this, &DSettingsDialog::close);
}

DSettingsDialog::~DSettingsDialog()
{

}

void DSettingsDialog::updateSettings(Dtk::Core::DSettings *settings)
{
    Q_D(DSettingsDialog);
    d->leftFrame->updateSettings(settings);
    d->content->updateSettings(settings);
    adjustSize();
}

}//DMUSIC_END_NAMESPACE
