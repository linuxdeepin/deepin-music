#include "closeconfirmdialog.h"

#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QBoxLayout>

#include "thememanager.h"
#include "core/settings.h"
class CloseConfirmDialogPrivate
{
public:
    CloseConfirmDialogPrivate(CloseConfirmDialog *parent) : q_ptr(parent) {}

    CloseConfirmDialog *q_ptr;
    Q_DECLARE_PUBLIC(CloseConfirmDialog)
};

CloseConfirmDialog::CloseConfirmDialog(QWidget *parent) :
    Dtk::Widget::DDialog(parent), d_ptr(new CloseConfirmDialogPrivate(this))
{
    Q_D(CloseConfirmDialog);

    ThemeManager::instance()->regisetrWidget(this);

    auto contentFrame = new QFrame;
    auto contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(0);

    auto groupLabel = new QLabel(tr("Please select your operation"));
    groupLabel->setObjectName("CloseConfirmDialogLabel");

    auto actionSelectionGroup = new QGroupBox();
    actionSelectionGroup->setContentsMargins(0, 0, 0, 0);
    actionSelectionGroup->setObjectName("CloseConfirmDialogSelectionGroup");

    auto exitBt = new QRadioButton(tr("Exit"));
    exitBt->setObjectName("CloseConfirmDialogExit");
    auto miniBt = new QRadioButton(tr("Minimize to system tray"));
    miniBt->setObjectName("CloseConfirmDialogMini");
    auto vbox = new QHBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(exitBt);
    vbox->addWidget(miniBt);
    vbox->addStretch(1);
    actionSelectionGroup->setLayout(vbox);

    auto remember = new QCheckBox(tr("Never ask again"));

    contentLayout->addWidget(groupLabel, 0, Qt::AlignLeft);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(actionSelectionGroup, 0, Qt::AlignLeft);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(remember, 0, Qt::AlignLeft);

    setIcon(QIcon(":/common/image/deepin-music.svg"));
    addContent(contentFrame);

    addButton(tr("Cancel"), false, ButtonNormal);
    addButton(tr("OK"), true, ButtonRecommend);

    remember->setChecked(!AppSettings::instance()->value("base.close.ask_close_action").toBool());
    if (1 == AppSettings::instance()->value("base.close.quit_action").toInt()) {
        exitBt->setChecked(true);
    } else {
        miniBt->setChecked(true);
    }

    connect(exitBt, &QRadioButton::clicked, this, [ = ]() {
        bool ask = !remember->isChecked();
        Q_EMIT quitAction(ask, 1);
    });
    connect(miniBt, &QRadioButton::clicked, this, [ = ]() {
        bool ask = !remember->isChecked();
        Q_EMIT quitAction(ask, 0);
    });
    connect(remember, &QCheckBox::toggled, this, [ = ]() {
        bool ask = !remember->isChecked();
        int action = exitBt->isChecked() ? 0 : 1;
        Q_EMIT quitAction(ask, action);
    });
}

CloseConfirmDialog::~CloseConfirmDialog()
{

}
