#include "dequalizerdialog.h"

#include <QPainter>
#include <QProxyStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DFrame>
#include <QDebug>

#include <DAbstractDialog>


#include <QToolTip>
#include <DLabel>
#include <DTitlebar>
#include <DSwitchButton>
#include <DTabWidget>
#include <DComboBox>
#include <DSlider>
#include <DBlurEffectWidget>
#include <DFloatingMessage>
#include <DPushButton>
#include <DTitlebar>



#include <vlc/Audio.h>
#include <vlc/Equalizer.h>
#include <vlc/MediaPlayer.h>
#include "ac-desktop-define.h"

DGUI_USE_NAMESPACE

class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
            s.transpose();
            s.rwidth() = 144; // 设置每个tabBar中item的大小
            s.rheight() = 30;
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel) {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

                if (tab->state & QStyle::State_Selected) {
                    painter->save();
                    painter->setPen(0x89cfff);
                    painter->setBrush(QBrush(0x89cfff));
                    painter->restore();
                }
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);
                if (tab->state & QStyle::State_Selected) {
                    painter->setPen(0xf8fcff);
                } else {
                    painter->setPen(0x5d5d5d);
                }

                painter->drawText(tab->rect, tab->text, option);
                return;
            }
        }

        if (element == CE_TabBarTab) {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
};

//开机后默认参数设置
void DequalizerDialog::readConfig()
{
//    Q_Q(DequalizerDialog);
    switchflag = MusicSettings::value("equalizer.all.switch").toBool();
    curEffect   = MusicSettings::value("equalizer.all.curEffect").toInt();

}

void DequalizerDialog::initUI()
{
    resize(720, 463);

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);
    font.setPixelSize(17);
    mtitleLabel  = new DLabel(DequalizerDialog::tr("Equalizer"));
    mtitleLabel->resize(51, 25);
    mtitleLabel->setFont(font);

    mswitchLabel = new DLabel;
    mswitchLabel->resize(14, 20);
    if (switchflag) {
        mswitchLabel->setText(DequalizerDialog::tr("ON"));
    } else {
        mswitchLabel->setText(DequalizerDialog::tr("OFF"));
    }

    mswitchBtn = new DSwitchButton(this);
    mswitchBtn->resize(50, 20);
    AC_SET_OBJECT_NAME(mswitchBtn, AC_switchBtn);
    AC_SET_ACCESSIBLE_NAME(mswitchBtn, AC_switchBtn);
//    //设置底色
//    DPalette sw;
//    sw.setColor(DPalette::Button,QColor("#000000"));
//    mswitchBtn->setPalette(sw);

    mcombox = new DComboBox(this);
    mcombox->setMinimumSize(240, 36);
    mcombox->addItems(effect_type);
    AC_SET_OBJECT_NAME(mcombox, AC_effectCombox);
    AC_SET_ACCESSIBLE_NAME(mcombox, AC_effectCombox);


    saveBtn = new DPushButton(this);
    saveBtn->setMinimumSize(110, 36);
    saveBtn->setText(DequalizerDialog::tr("Save"));
    AC_SET_OBJECT_NAME(saveBtn, AC_saveBtn);
    AC_SET_ACCESSIBLE_NAME(saveBtn, AC_saveBtn);

    lb_baud_pre = new DLabel(this);
    lb_baud_pre->setMaximumWidth(20);
    lb_baud_pre->setMinimumHeight(22);
    lb_baud_pre->setAlignment(Qt::AlignCenter);
    slider_pre = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_pre, AC_baud_pre);
    AC_SET_ACCESSIBLE_NAME(slider_pre, AC_baud_pre);
    slider_pre->setMinimum(-20);
    slider_pre->setMaximum(20);
    slider_pre->setPageStep(1);
    auto lb_preamplifier = new DLabel(DequalizerDialog::tr("Preamplifier"));
    lb_preamplifier->setAlignment(Qt::AlignCenter);

    lb_baud_60 = new DLabel(this);
    lb_baud_60->setMaximumWidth(20);
    lb_baud_60->setMinimumHeight(22);
    lb_baud_60->setAlignment(Qt::AlignCenter);
    slider_60 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_60, AC_slider_60);
    AC_SET_ACCESSIBLE_NAME(slider_60, AC_slider_60);
    slider_60->setMinimum(-20);
    slider_60->setMaximum(20);
    slider_60->setPageStep(1);
    auto lb_60     = new DLabel("60");

    lb_baud_170 = new DLabel(this);
    lb_baud_170->setMaximumWidth(20);
    lb_baud_170->setMinimumHeight(22);
    lb_baud_170->setAlignment(Qt::AlignCenter);
    slider_170 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_170, AC_slider_170);
    AC_SET_ACCESSIBLE_NAME(slider_170, AC_slider_170);
    slider_170->setMinimum(-20);
    slider_170->setMaximum(20);
    slider_170->setPageStep(1);
    auto lb_170     = new DLabel("170");

    lb_baud_310 = new DLabel(this);
    lb_baud_310->setMaximumWidth(20);
    lb_baud_310->setMinimumHeight(22);
    lb_baud_310->setAlignment(Qt::AlignCenter);
    slider_310 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_310, AC_slider_310);
    AC_SET_ACCESSIBLE_NAME(slider_310, AC_slider_310);
    slider_310->setMinimum(-20);
    slider_310->setMaximum(20);
    slider_310->setPageStep(1);
    auto lb_310     = new DLabel("310");

    lb_baud_600 = new DLabel(this);
    lb_baud_600->setMaximumWidth(20);
    lb_baud_600->setMinimumHeight(22);
    lb_baud_600->setAlignment(Qt::AlignCenter);
    slider_600 = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_600, AC_slider_600);
    AC_SET_ACCESSIBLE_NAME(slider_600, AC_slider_600);
    slider_600->setMinimum(-20);
    slider_600->setMaximum(20);
    slider_600->setPageStep(1);
    auto lb_600     = new DLabel("600");

    lb_baud_1K = new DLabel(this);
    lb_baud_1K->setMaximumWidth(20);
    lb_baud_1K->setMinimumHeight(22);
    lb_baud_1K->setAlignment(Qt::AlignCenter);
    slider_1K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_1K, AC_slider_1K);
    AC_SET_ACCESSIBLE_NAME(slider_1K, AC_slider_1K);
    slider_1K->setMinimum(-20);
    slider_1K->setMaximum(20);
    slider_1K->setPageStep(1);
    auto lb_1K     = new DLabel("1K");

    lb_baud_3K = new DLabel(this);
    lb_baud_3K->setMaximumWidth(20);
    lb_baud_3K->setMinimumHeight(22);
    lb_baud_3K->setAlignment(Qt::AlignCenter);
    slider_3K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_3K, AC_slider_3K);
    AC_SET_ACCESSIBLE_NAME(slider_3K, AC_slider_3K);
    slider_3K->setMinimum(-20);
    slider_3K->setMaximum(20);
    slider_3K->setPageStep(1);
    auto lb_3K     = new DLabel("3K");

    lb_baud_6K = new DLabel(this);
    lb_baud_6K->setMaximumWidth(20);
    lb_baud_6K->setMinimumHeight(22);
    lb_baud_6K->setAlignment(Qt::AlignCenter);
    slider_6K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_6K, AC_slider_6K);
    AC_SET_ACCESSIBLE_NAME(slider_6K, AC_slider_6K);
    slider_6K->setMinimum(-20);
    slider_6K->setMaximum(20);
    slider_6K->setPageStep(1);
    auto lb_6K    = new DLabel("6K");

    lb_baud_12K = new DLabel(this);
    lb_baud_12K->setMaximumWidth(20);
    lb_baud_12K->setMinimumHeight(22);
    lb_baud_12K->setAlignment(Qt::AlignCenter);
    slider_12K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_12K, AC_slider_12K);
    AC_SET_ACCESSIBLE_NAME(slider_12K, AC_slider_12K);
    slider_12K->setMinimum(-20);
    slider_12K->setMaximum(20);
    slider_12K->setPageStep(1);
    auto lb_12K     = new DLabel("12K");

    lb_baud_14K = new DLabel(this);
    lb_baud_14K->setMaximumWidth(20);
    lb_baud_14K->setMinimumHeight(22);
    lb_baud_14K->setAlignment(Qt::AlignCenter);
    slider_14K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_14K, AC_slider_14K);
    AC_SET_ACCESSIBLE_NAME(slider_14K, AC_slider_14K);
    slider_14K->setMinimum(-20);
    slider_14K->setMaximum(20);
    slider_14K->setPageStep(1);
    auto lb_14K     = new DLabel("14K");

    lb_baud_16K = new DLabel(this);
    lb_baud_16K->setMaximumWidth(20);
    lb_baud_16K->setMinimumHeight(22);
    lb_baud_16K->setAlignment(Qt::AlignCenter);
    slider_16K = new DSlider(Qt::Vertical);
    AC_SET_OBJECT_NAME(slider_16K, AC_slider_16K);
    AC_SET_ACCESSIBLE_NAME(slider_16K, AC_slider_16K);
    slider_16K->setMinimum(-20);
    slider_16K->setMaximum(20);
    slider_16K->setPageStep(1);
    auto lb_16K     = new DLabel("16K");

    btn_default = new DPushButton(DequalizerDialog::tr("Restore Defaults"));
    btn_default->setMinimumSize(240, 36);
    AC_SET_OBJECT_NAME(btn_default, AC_Restore);
    AC_SET_ACCESSIBLE_NAME(btn_default, AC_Restore);

    auto vlayout0  = new QVBoxLayout;
    auto vlayout1  = new QVBoxLayout;
    auto vlayout2  = new QVBoxLayout;
    auto vlayout3  = new QVBoxLayout;
    auto vlayout4  = new QVBoxLayout;
    auto vlayout5  = new QVBoxLayout;
    auto vlayout6  = new QVBoxLayout;
    auto vlayout7  = new QVBoxLayout;
    auto vlayout8  = new QVBoxLayout;
    auto vlayout9  = new QVBoxLayout;
    auto vlayout10 = new QVBoxLayout;

    vlayout0->addWidget(lb_baud_pre);
    vlayout0->addWidget(slider_pre);
    vlayout0->addWidget(lb_preamplifier);

    vlayout1->addWidget(lb_baud_60);
    vlayout1->addWidget(slider_60);
    vlayout1->addWidget(lb_60);

    vlayout2->addWidget(lb_baud_170);
    vlayout2->addWidget(slider_170);
    vlayout2->addWidget(lb_170);

    vlayout3->addWidget(lb_baud_310);
    vlayout3->addWidget(slider_310);
    vlayout3->addWidget(lb_310);

    vlayout4->addWidget(lb_baud_600);
    vlayout4->addWidget(slider_600);
    vlayout4->addWidget(lb_600);

    vlayout5->addWidget(lb_baud_1K);
    vlayout5->addWidget(slider_1K);
    vlayout5->addWidget(lb_1K);

    vlayout6->addWidget(lb_baud_3K);
    vlayout6->addWidget(slider_3K);
    vlayout6->addWidget(lb_3K);

    vlayout7->addWidget(lb_baud_6K);
    vlayout7->addWidget(slider_6K);
    vlayout7->addWidget(lb_6K);

    vlayout8->addWidget(lb_baud_12K);
    vlayout8->addWidget(slider_12K);
    vlayout8->addWidget(lb_12K);

    vlayout9->addWidget(lb_baud_14K);
    vlayout9->addWidget(slider_14K);
    vlayout9->addWidget(lb_14K);

    vlayout10->addWidget(lb_baud_16K);
    vlayout10->addWidget(slider_16K);
    vlayout10->addWidget(lb_16K);

    auto hlayout  = new QHBoxLayout;
    hlayout->addLayout(vlayout0);
    hlayout->addLayout(vlayout1);
    hlayout->addLayout(vlayout2);
    hlayout->addLayout(vlayout3);
    hlayout->addLayout(vlayout4);
    hlayout->addLayout(vlayout5);
    hlayout->addLayout(vlayout6);
    hlayout->addLayout(vlayout7);
    hlayout->addLayout(vlayout8);
    hlayout->addLayout(vlayout9);
    hlayout->addLayout(vlayout10);
    hlayout->setSpacing(5);

    slWidget = new DBlurEffectWidget;
    slWidget->setBlurRectXRadius(8);
    slWidget->setBlurRectYRadius(8);
    slWidget->setRadius(8);
    slWidget->setBlurEnabled(true);
    slWidget->setMode(DBlurEffectWidget::GaussianBlur);
    slWidget->setMinimumWidth(517);
    slWidget->setMinimumHeight(264);
    slWidget->setLayout(hlayout);
    slWidget->setContentsMargins(10, 10, 9, 20);
    AC_SET_OBJECT_NAME(slWidget, AC_slWidget);
    AC_SET_ACCESSIBLE_NAME(slWidget, AC_slWidget);

    auto vlay  = new QVBoxLayout;
    //第一行
    vlay->addWidget(mtitleLabel);
    //第二行
    auto hlay  = new QHBoxLayout;
    hlay->addWidget(mswitchLabel, Qt::AlignLeft);
    hlay->addWidget(mswitchBtn, Qt::AlignLeft);
    hlay->addWidget(mcombox, 20, Qt::AlignCenter);
    hlay->addWidget(saveBtn, Qt::AlignRight);

    //第三行
    vlay->addLayout(hlay);
    vlay->addStretch(2);
    vlay->addWidget(slWidget);
    vlay->addWidget(btn_default, 0, Qt::AlignCenter);
    vlay->setContentsMargins(20, 10, 20, 20);

    //均衡器页面
    auto mequalizer = new DWidget(this);
    mequalizer->resize(537, 393);
    mequalizer->setLayout(vlay);
    AC_SET_OBJECT_NAME(mequalizer, AC_mequalizer);
    AC_SET_ACCESSIBLE_NAME(mequalizer, AC_mequalizer);

    auto mtabwidget = new DTabWidget;
    AC_SET_OBJECT_NAME(mtabwidget, AC_mtabwidget);
    AC_SET_ACCESSIBLE_NAME(mtabwidget, AC_mtabwidget);
    mtabwidget->setTabPosition(QTabWidget::West);
    mtabwidget->setAutoFillBackground(true);
    mtabwidget->setDocumentMode(true);
    mtabwidget->resize(720, 463);
    mtabwidget->addTab(mequalizer, DequalizerDialog::tr("Equalizer"));
    tabStyle = new CustomTabStyle;
    mtabwidget->tabBar()->setStyle(tabStyle);
    mtabwidget->tabBar()->setContentsMargins(10, 0, 10, 0);

    auto mTitlebar = new DTitlebar(this);
    mTitlebar->setTitle("");
    mTitlebar->resize(720, 50);
    AC_SET_OBJECT_NAME(mTitlebar, AC_mTitlebar);
    AC_SET_ACCESSIBLE_NAME(mTitlebar, AC_mTitlebar);

    //垂直布局TabWidget和TitleBar
    auto  mlayout = new QVBoxLayout;
    mlayout->addWidget(mTitlebar);
    mlayout->addWidget(mtabwidget);
    mlayout->setSpacing(0);
    mlayout->setMargin(0);
    this->setLayout(mlayout);

    AllbaudTypes.append(flat_bauds);
    AllbaudTypes.append(Classical_bauds);
    AllbaudTypes.append(Club_bauds);
    AllbaudTypes.append(Dance_bauds);
    AllbaudTypes.append(Full_bass_bauds);
    AllbaudTypes.append(Full_bass_and_treble_bauds);
    AllbaudTypes.append(Full_treble_bauds);
    AllbaudTypes.append(Headphones_bauds);
    AllbaudTypes.append(Large_Hall_bauds);
    AllbaudTypes.append(Live_bauds);
    AllbaudTypes.append(Party_bauds);
    AllbaudTypes.append(Pop_bauds);
    AllbaudTypes.append(Reggae_bauds);
    AllbaudTypes.append(Rock_bauds);
    AllbaudTypes.append(Ska_bauds);
    AllbaudTypes.append(Soft_bauds);
    AllbaudTypes.append(Soft_rock_bauds);
    AllbaudTypes.append(Techno_bauds);

    mswitchBtn->setChecked(switchflag);
    mcombox->setEnabled(switchflag);
    saveBtn->setEnabled(switchflag);

    for (DSlider *slider : this->findChildren<DSlider *>()) {
        slider->setAttribute(Qt::WA_Hover, true); //开启悬停事件
        slider->installEventFilter(this);       //安装事件过滤器
        slider->setEnabled(switchflag);
    }
    if (curEffect != 0) {
        this->showCurMode(AllbaudTypes.at(curEffect - 1));
        mcombox->setCurrentIndex(curEffect);
    } else {
        this->showCustom();
    }
}

void DequalizerDialog::SliderOneEntry(DSlider *slider, int value)
{
    this->selectSlider(slider, QString::number(value));
    if (slider == slider_pre) {
        Q_EMIT this->setEqualizerpre(value);
    } else {
        int bandIndex = -1;
        if (slider == slider_60) {
            bandIndex = 0;
        } else if (slider == slider_170) {
            bandIndex = 1;
        } else if (slider == slider_310) {
            bandIndex = 2;
        } else if (slider == slider_600) {
            bandIndex = 3;
        } else if (slider == slider_1K) {
            bandIndex = 4;
        } else if (slider == slider_3K) {
            bandIndex = 5;
        } else if (slider == slider_6K) {
            bandIndex = 6;
        } else if (slider == slider_12K) {
            bandIndex = 7;
        } else if (slider == slider_14K) {
            bandIndex = 8;
        } else if (slider == slider_16K) {
            bandIndex = 9;
        }
        if (bandIndex == -1) {
            return;
        }
        Q_EMIT this->setEqualizerbauds(bandIndex, value);
    }
}


DequalizerDialog::DequalizerDialog(QWidget *parent):
    DAbstractDialog(parent)
{
    AC_SET_OBJECT_NAME(this, AC_Dequalizer);
    AC_SET_ACCESSIBLE_NAME(this, AC_Dequalizer);
    effect_type << DequalizerDialog::tr("Custom")
                << DequalizerDialog::tr("Monophony")
                << DequalizerDialog::tr("Classical")
                << DequalizerDialog::tr("Club")
                << DequalizerDialog::tr("Dance")
                << DequalizerDialog::tr("Full Bass")
                << DequalizerDialog::tr("Full Bass and Treble")
                << DequalizerDialog::tr("Full Treble")
                << DequalizerDialog::tr("Headphones")
                << DequalizerDialog::tr("Hall")
                << DequalizerDialog::tr("Live")
                << DequalizerDialog::tr("Party")
                << DequalizerDialog::tr("Pop")
                << DequalizerDialog::tr("Reggae")
                << DequalizerDialog::tr("Rock")
                << DequalizerDialog::tr("Ska")
                << DequalizerDialog::tr("Soft")
                << DequalizerDialog::tr("Soft Rock")
                << DequalizerDialog::tr("Techno");
    this->readConfig();
    this->initUI();
    initConnection();
    QFont font;
    font.setPixelSize(13);
    QFontMetrics fm(font);
    this->saveMessage  = new DFloatingMessage(DFloatingMessage::TransientType, this);
    this->saveMessage->setFont(font);
    this->saveMessage->setIcon(QIcon(":/common/image/notify_success_new.svg"));
    this->saveMessage->setMessage(tr("Sound Effects Saved"));
    int Minwid = fm.width(tr("Sound Effects Saved")) + 70;
    this->saveMessage->setMinimumSize(Minwid, 60);
    this->saveMessage->setDuration(2000);
    this->saveMessage->move(width() / 2 - 80, height() - 70);
    this->saveMessage->hide();
}

DequalizerDialog::~DequalizerDialog()
{
    delete this->saveMessage;
    delete this->tabStyle;
}

void DequalizerDialog::initConnection()
{
    connect(this->mswitchBtn, &DSwitchButton::checkedChanged, this, &DequalizerDialog::checkedChanged);

    for (DSlider *slider : findChildren<DSlider *>()) {
        connect(slider, &DSlider::sliderReleased, [ = ]() {
            this->changeflag = true;
            if (this->mcombox->currentIndex() != 0) {
                this->mcombox->setCurrentIndex(0);
            }
            int value = slider->value();
            this->SliderOneEntry(slider, value);
        });

        connect(slider, &DSlider::valueChanged, [ = ](int value) {
            this->saveBtn->setEnabled(true);

            // 音乐模式切换到自定义模式，应该进行值的变化显示
            this->SliderOneEntry(slider, value);
        });
    }

    connect(this->saveBtn, &DPushButton::clicked, [ = ]() {
        for (DSlider *slider : findChildren<DSlider *>()) {
            MusicSettings::setOption("equalizer.all." + slider->objectName(), slider->value());
        }
        MusicSettings::setOption("equalizer.all.curEffect", 0);
        this->saveMessage->show();
    });

    connect(this->mcombox, QOverload<int>::of(&DComboBox::currentIndexChanged),
    this, [ = ](int index) {
        if (index == 0) {
            showCustom();
        } else {
            this->changeflag = false;
            showCurMode(this->AllbaudTypes.at(index - 1));
            Q_EMIT setEqualizerIndex(index);
            MusicSettings::setOption("equalizer.all.curEffect", index);
        }
    });
    connect(this->btn_default, &DPushButton::clicked, this, &DequalizerDialog::setDefaultClicked);

}
//显示自定义模式
void DequalizerDialog::showCustom()
{
    if (this->changeflag) {
        return;
    }

    for (DSlider *slider : findChildren<DSlider *>()) {
        int indexbaud = MusicSettings::value("equalizer.all." + slider->objectName()).toInt();
        slider->setValue(indexbaud);
        selectSlider(slider, "");
    }
    MusicSettings::setOption("equalizer.all.curEffect", 0);
}

//显示非自定义模式
void DequalizerDialog::showCurMode(int *Allbauds)
{
    if (Allbauds == nullptr) {
        return;
    }
    int i = 0;
    for (DSlider *slider : findChildren<DSlider *>()) {
        if (this->mswitchBtn->isChecked()) {
            slider->setEnabled(true);
        }
        slider->setValue(Allbauds[i]);
        selectSlider(slider, "");
        i++;
    }
    this->saveBtn->setEnabled(false);
}

//开关
void DequalizerDialog::checkedChanged(bool checked)
{
    for (DSlider *slider : findChildren<DSlider *>()) {
        slider->setEnabled(checked);
    }
    this->mswitchLabel->setText(tr("OFF"));
    if (checked) {
        this->mswitchLabel->setText(tr("ON"));
        //flat;
        showCurMode(this->flat_bauds);
        this->mcombox->setCurrentIndex(1);
        Q_EMIT setEqualizerIndex(1);
        MusicSettings::setOption("equalizer.all.curEffect", 1);
    } else {
        Q_EMIT setEqualizerIndex(1);
        MusicSettings::setOption("equalizer.all.curEffect", 1);
    }
    this->mcombox->setEnabled(checked);
    this->saveBtn->setEnabled(false);
//    Q_EMIT setEqualizerEnable(checked);
    MusicSettings::setOption("equalizer.all.switch", checked);
}

//恢复默认
void DequalizerDialog::setDefaultClicked()
{
    if (this->mswitchBtn->isChecked()) {
        showCurMode(this->flat_bauds);
        this->mswitchBtn->setChecked(false);
        this->mcombox->setCurrentIndex(1);
        MusicSettings::setOption("equalizer.all.curEffect", 1);
        for (DSlider *slider : findChildren<DSlider *>()) {
            if (slider == this->slider_pre) {
                MusicSettings::setOption("equalizer.all." + slider->objectName(), 12);
            } else {
                MusicSettings::setOption("equalizer.all." + slider->objectName(), 0);
            }
        }
    }
}
void DequalizerDialog::selectSlider(QObject *obj, QString SliderVal)
{
    if (obj == this->slider_pre) {
        this->lb_baud_pre->setText(SliderVal);
    } else if (obj == this->slider_60) {
        this->lb_baud_60->setText(SliderVal);
    } else if (obj == this->slider_170) {
        this->lb_baud_170->setText(SliderVal);
    } else if (obj == this->slider_310) {
        this->lb_baud_310->setText(SliderVal);
    } else if (obj == this->slider_600) {
        this->lb_baud_600->setText(SliderVal);
    } else if (obj == this->slider_1K) {
        this->lb_baud_1K->setText(SliderVal);
    } else if (obj == this->slider_3K) {
        this->lb_baud_3K->setText(SliderVal);
    } else if (obj == this->slider_6K) {
        this->lb_baud_6K->setText(SliderVal);
    } else if (obj == this->slider_12K) {
        this->lb_baud_12K->setText(SliderVal);
    } else if (obj == this->slider_14K) {
        this->lb_baud_14K->setText(SliderVal);
    } else if (obj == this->slider_16K) {
        this->lb_baud_16K->setText(SliderVal);
    }
}

bool DequalizerDialog::eventFilter(QObject *obj, QEvent *event)
{
    for (DSlider *slider : findChildren<DSlider *>()) {
        if (obj == slider) {
            if (event->type() == QEvent::HoverEnter) {
                selectSlider(slider, QString::number(slider->value()));
                return true;
            }
            if (event->type() == QEvent::HoverLeave) {
                selectSlider(slider, "");
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
