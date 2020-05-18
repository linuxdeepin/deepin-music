#include "dequalizerdialog.h"

#include <QPainter>
#include <QProxyStyle>
#include <DLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DSlider>
#include <DBlurEffectWidget>
#include <DTabWidget>
#include <DSwitchButton>
#include <DComboBox>
#include <DTitlebar>
#include <DFrame>
#include <DPushButton>
#include <QDebug>
#include <QMap>

#include <vlc/Audio.h>
#include <vlc/Equalizer.h>
#include <vlc/MediaPlayer.h>


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

DWIDGET_USE_NAMESPACE
class DequalizerDialogPrivate
{
public:
    DequalizerDialogPrivate(DequalizerDialog *parent) : q_ptr(parent) {}
    void readConfig();
    void initUI();

private:
    DTitlebar   *mtitlebar            = nullptr;
    DSwitchButton *mswitchBtn         = nullptr;
    DComboBox      *mcombox           = nullptr;
    DPushButton    *saveBtn           = nullptr;

    DLabel      *mtitleLabel          = nullptr;
    DLabel      *mswitchLabel         = nullptr;
    DLabel      *lb_baud_pre          = nullptr;
    DLabel      *lb_baud_60           = nullptr;
    DLabel      *lb_baud_170          = nullptr;
    DLabel      *lb_baud_310          = nullptr;
    DLabel      *lb_baud_600          = nullptr;
    DLabel      *lb_baud_1K           = nullptr;
    DLabel      *lb_baud_3K           = nullptr;
    DLabel      *lb_baud_6K           = nullptr;
    DLabel      *lb_baud_12K          = nullptr;
    DLabel      *lb_baud_14K          = nullptr;
    DLabel      *lb_baud_16K          = nullptr;

    DSlider     *slider_pre           = nullptr;
    DSlider     *slider_60            = nullptr;
    DSlider     *slider_170           = nullptr;
    DSlider     *slider_310           = nullptr;
    DSlider     *slider_600           = nullptr;
    DSlider     *slider_1K            = nullptr;
    DSlider     *slider_3K            = nullptr;
    DSlider     *slider_6K            = nullptr;
    DSlider     *slider_12K           = nullptr;
    DSlider     *slider_14K           = nullptr;
    DSlider     *slider_16K           = nullptr;
    DPushButton *btn_default          = nullptr;

    QStringList    effect_type        = {"自定义", "单调", "古典", "俱乐部", "舞曲", "全低音",
                                         "全低音和高音", "全高音", "耳机", "大厅", "实况", "聚会",
                                         "流行", "雷盖", "摇滚", "斯卡", "柔和", "慢摇", "电子乐"
                                        };
    QList<int> BaudList;

    MusicSettings   *settings         = nullptr;


    bool    switch_flag               = false;
    bool    changeflag                = true;
    int     curEffect                 = 0;


    int flat_bauds[11] = {12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int Classical_bauds[11] = {12, 0, 0, 0, 0, 0, 0, -7, -7, -7, -9};
    int Club_bauds[11] = {12, 0, 0, 0, 0, 0, 0, -7, -7, -7, -9};
    int Dance_bauds[11] = {5, 9, 7, 2, 0, 0, -5, -7, -7, 0, 0};
    int Full_bass_bauds[11] = {5, -8, 9, 9, 5, 1, -4, -8, -10, -11, -11};
    int Full_bass_and_treble_bauds[11] = {4, 7, 5, 0, -7, -4, 1, 8, 11, 12, 12};
    int Full_treble_bauds[11] = {3, -9, -9, -9, -4, 2, 11, 16, 16, 16, 16};
    int Headphones_bauds[11] = {4, 4, 11, 5, -3, -2, 1, 4, 9, 12, 14};
    int Large_Hall_bauds[11] = {5, 10, 10, 5, 5, 0, -4, -4, -4, 0, 0};
    int Live_bauds[11] = {7, -4, 0, 4, 5, 5, 5, 4, 2, 2, 2};
    int Party_bauds[11] = {6, 7, 7, 0, 0, 0, 0, 0, 0, 7, 7};
    int Pop_bauds[11] = {6, -1, 4, 7, 8, 5, 0, -2, -2, -1, -1};
    int Reggae_bauds[11] = {8, 0, 0, 0, -5, 0, 6, 6, 0, 0, 0};
    int Rock_bauds[11] = {5, 8, 4, -5, -8, -3, 4, 8, 11, 11, 11};
    int Ska_bauds[11] = {6, -2, -4, -4, 0, 4, 5, 8, 9, 11, 9};
    int Soft_bauds[11] = {5, 4, 1, 0, -2, 0, 4, 8, 9, 11, 12};
    int Soft_rock_bauds[11] = {7, 4, 4, 2, 0, -4, -5, -3, 0, 2, 8};
    int Techno_bauds[11] = {5, 8, 5, 0, -5, -4, 0, 8, 9, 9, 8};

    QList<int *> AllbaudTypes;
    //slidergroup
    DBlurEffectWidget *slWidget  = nullptr;
    DequalizerDialog *q_ptr;

    Q_DECLARE_PUBLIC(DequalizerDialog)
};

//开机后默认参数设置
void DequalizerDialogPrivate::readConfig()
{
//    Q_Q(DequalizerDialog);
    switch_flag = settings->value("equalizer.all.switch").toBool();
    curEffect   = settings->value("equalizer.all.curEffect").toInt();

}

void DequalizerDialogPrivate::initUI()
{
    Q_Q(DequalizerDialog);

    q->resize(720, 463);

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);
    font.setPixelSize(17);
    mtitleLabel  = new DLabel("均衡器");
    mtitleLabel->resize(51, 25);
    mtitleLabel->setFont(font);

    mswitchLabel = new DLabel;
    mswitchLabel->resize(14, 20);
    if (switch_flag) {
        mswitchLabel->setText("开");
    } else {
        mswitchLabel->setText("关");
    }

    mswitchBtn = new DSwitchButton(q);
    mswitchBtn->resize(50, 20);
//    //设置底色
//    DPalette sw;
//    sw.setColor(DPalette::Button,QColor("#000000"));
//    mswitchBtn->setPalette(sw);

    mcombox = new DComboBox(q);
    mcombox->setMinimumSize(240, 36);
    mcombox->addItems(effect_type);


    saveBtn = new DPushButton(q);
    saveBtn->setMinimumSize(110, 36);
    saveBtn->setText("保存");

    slider_pre = new DSlider(Qt::Vertical);
    slider_pre->setObjectName("baud_pre");
    slider_pre->setMinimum(-20);
    slider_pre->setMaximum(20);
    slider_pre->setPageStep(1);
    auto lb_preamplifier = new DLabel("前置放大");
    lb_baud_pre = new DLabel(q);

    slider_60 = new DSlider(Qt::Vertical);
    slider_60->setObjectName("baud_60");
    slider_60->setMinimum(-20);
    slider_60->setMaximum(20);
    slider_60->setPageStep(1);
    auto lb1     = new DLabel("60");
    lb_baud_60 = new DLabel(q);

    slider_170 = new DSlider(Qt::Vertical);
    slider_170->setObjectName("baud_170");
    slider_170->setMinimum(-20);
    slider_170->setMaximum(20);
    slider_170->setPageStep(1);
    auto lb2     = new DLabel("170");
    lb_baud_170 = new DLabel(q);

    slider_310 = new DSlider(Qt::Vertical);
    slider_310->setObjectName("baud_310");
    slider_310->setMinimum(-20);
    slider_310->setMaximum(20);
    slider_310->setPageStep(1);
    auto lb3     = new DLabel("310");
    lb_baud_310 = new DLabel(q);

    slider_600 = new DSlider(Qt::Vertical);
    slider_600->setObjectName("baud_600");
    slider_600->setMinimum(-20);
    slider_600->setMaximum(20);
    slider_600->setPageStep(1);
    auto lb4     = new DLabel("600");
    lb_baud_600 = new DLabel(q);

    slider_1K = new DSlider(Qt::Vertical);
    slider_1K->setObjectName("baud_1K");
    slider_1K->setMinimum(-20);
    slider_1K->setMaximum(20);
    slider_1K->setPageStep(1);
    auto lb5     = new DLabel("1K");
    lb_baud_1K = new DLabel(q);

    slider_3K = new DSlider(Qt::Vertical);
    slider_3K->setObjectName("baud_3K");
    slider_3K->setMinimum(-20);
    slider_3K->setMaximum(20);
    slider_3K->setPageStep(1);

    auto lb6     = new DLabel("3K");
    lb_baud_3K = new DLabel(q);

    slider_6K = new DSlider(Qt::Vertical);
    slider_6K->setObjectName("baud_6K");
    slider_6K->setMinimum(-20);
    slider_6K->setMaximum(20);
    slider_6K->setPageStep(1);
    auto lb7    = new DLabel("6K");
    lb_baud_6K = new DLabel(q);

    slider_12K = new DSlider(Qt::Vertical);
    slider_12K->setObjectName("baud_12K");
    slider_12K->setMinimum(-20);
    slider_12K->setMaximum(20);
    slider_12K->setPageStep(1);
    auto lb8     = new DLabel("12K");
    lb_baud_12K = new DLabel(q);

    slider_14K = new DSlider(Qt::Vertical);
    slider_14K->setObjectName("baud_14K");
    slider_14K->setMinimum(-20);
    slider_14K->setMaximum(20);
    slider_14K->setPageStep(1);
    auto lb9     = new DLabel("14K");
    lb_baud_14K = new DLabel(q);

    slider_16K = new DSlider(Qt::Vertical);
    slider_16K->setObjectName("baud_16K");
    slider_16K->setMinimum(-20);
    slider_16K->setMaximum(20);
    slider_16K->setPageStep(1);
    auto lb10     = new DLabel("16K");
    lb_baud_16K = new DLabel(q);

    btn_default = new DPushButton("恢复默认");
    btn_default->setMinimumSize(240, 36);

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

//    vlayout0->addWidget(lb_baud_pre);
    vlayout0->addWidget(slider_pre, Qt::AlignCenter);
    vlayout0->addWidget(lb_preamplifier);

//    vlayout1->addWidget(lb_baud_60);
    vlayout1->addWidget(slider_60);
    vlayout1->addWidget(lb1);

//    vlayout2->addWidget(lb_baud_170);
    vlayout2->addWidget(slider_170);
    vlayout2->addWidget(lb2);

//    vlayout3->addWidget(lb_baud_310);
    vlayout3->addWidget(slider_310);
    vlayout3->addWidget(lb3);

//    vlayout4->addWidget(lb_baud_600);
    vlayout4->addWidget(slider_600);
    vlayout4->addWidget(lb4);

//    vlayout5->addWidget(lb_baud_1K);
    vlayout5->addWidget(slider_1K);
    vlayout5->addWidget(lb5);

//    vlayout6->addWidget(lb_baud_3K);
    vlayout6->addWidget(slider_3K);
    vlayout6->addWidget(lb6);

//    vlayout7->addWidget(lb_baud_6K);
    vlayout7->addWidget(slider_6K);
    vlayout7->addWidget(lb7);

//    vlayout8->addWidget(lb_baud_12K);
    vlayout8->addWidget(slider_12K);
    vlayout8->addWidget(lb8);

//    vlayout9->addWidget(lb_baud_14K);
    vlayout9->addWidget(slider_14K);
    vlayout9->addWidget(lb9);

//    vlayout10->addWidget(lb_baud_16K);
    vlayout10->addWidget(slider_16K);
    vlayout10->addWidget(lb10);

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
    auto mequalizer = new DWidget(q);
    mequalizer->resize(537, 393);
    mequalizer->setLayout(vlay);

    auto mtabwidget = new DTabWidget;
    mtabwidget->setTabPosition(QTabWidget::West);
    mtabwidget->setAutoFillBackground(true);
    mtabwidget->setDocumentMode(true);
    mtabwidget->resize(720, 463);
    mtabwidget->addTab(mequalizer, "均衡器");
    mtabwidget->tabBar()->setStyle(new CustomTabStyle);
    mtabwidget->tabBar()->setContentsMargins(10, 0, 10, 0);

    auto mTitlebar = new DTitlebar(q);
    mTitlebar->setTitle("");
    mTitlebar->resize(720, 50);

    //垂直布局TabWidget和TitleBar
    auto  mlayout = new QVBoxLayout;
    mlayout->addWidget(mTitlebar);
    mlayout->addWidget(mtabwidget);
    mlayout->setSpacing(0);
    mlayout->setMargin(0);
    q->setLayout(mlayout);

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

    mswitchBtn->setChecked(switch_flag);
    mcombox->setEnabled(switch_flag);

    for (DSlider *slider : q->findChildren<DSlider *>()) {
//        DLabel *valueLabel = q->findChild<DLabel *>(q->tr("lb_%1").arg(slider->objectName()));
//        q->connect(slider, &DSlider::valueChanged, q, [ = ](int value) {
//            valueLabel->setText(q->tr("%1 dB").arg(value));
//        });
        slider->setEnabled(switch_flag);
    }

    if (curEffect != 0 ) {
        q->showCurMode(AllbaudTypes.at(curEffect - 1));
        mcombox->setCurrentIndex(curEffect);
    } else {
        q->showCustom();
    }

}


DequalizerDialog::DequalizerDialog(QWidget *parent):
    DAbstractDialog(parent), d_ptr(new DequalizerDialogPrivate(this))
{
    Q_D(DequalizerDialog);
    d->readConfig();
    d->initUI();
    initConnection();
}

DequalizerDialog::~DequalizerDialog()
{
//    Q_D(DequalizerDialog);
}

void DequalizerDialog::initConnection()
{
    Q_D(DequalizerDialog);

    connect(d->mswitchBtn, &DSwitchButton::checkedChanged, this, &DequalizerDialog::checkedChanged);

    for (DSlider *slider : findChildren<DSlider *>()) {
        connect(slider, &DSlider::sliderReleased, [ = ]() {
            d->changeflag = false;
            d->mcombox->setCurrentIndex(0);
        });
        connect(slider, &DSlider::valueChanged, [ = ](int value) {
            if (slider == d->slider_pre) {
                Q_EMIT setEqualizerpre(value);
            } else {
                int bandIndex = -1;
                if (slider == d->slider_60) {
                    bandIndex = 0;
                } else if (slider == d->slider_170) {
                    bandIndex = 1;
                } else if (slider == d->slider_310) {
                    bandIndex = 2;
                } else if (slider == d->slider_600) {
                    bandIndex = 3;
                } else if (slider == d->slider_1K) {
                    bandIndex = 4;
                } else if (slider == d->slider_3K) {
                    bandIndex = 5;
                } else if (slider == d->slider_6K) {
                    bandIndex = 6;
                } else if (slider == d->slider_12K) {
                    bandIndex = 7;
                } else if (slider == d->slider_14K) {
                    bandIndex = 8;
                } else if (slider == d->slider_16K) {
                    bandIndex = 9;
                }
                if (bandIndex == -1) {
                    return;
                }
                Q_EMIT setEqualizerbauds(bandIndex, value);
            }
            d->saveBtn->setEnabled(true);
        });
    }
    connect(d->saveBtn, &DPushButton::clicked, [ = ]() {
        for (DSlider *slider : findChildren<DSlider *>()) {
            d->settings->setOption(tr("equalizer.all.%1").arg( slider->objectName()), slider->value());
            qDebug() << "save" << slider->value();
        }
    });

    connect(d->mcombox, QOverload<int>::of(&DComboBox::currentIndexChanged),
    this, [ = ](int index) {
        qDebug() << "index:" << index;
        if (index == 0) {
            //自定义模式
            showCustom();
        } else {
            d->changeflag = true;
            showCurMode(d->AllbaudTypes.at(index - 1));
            Q_EMIT setEqualizerIndex(index);
        }
        d->settings->setOption("equalizer.all.curEffect", index);
    });
    connect(d->btn_default, &DPushButton::clicked, this, &DequalizerDialog::setDefaultClicked);

}
//显示自定义模式
void DequalizerDialog::showCustom()
{
    Q_D(DequalizerDialog);
    if (d->changeflag) {
        for (DSlider *slider : findChildren<DSlider *>()) {
            int indexbaud = d->settings->value(tr("equalizer.all.%1").arg(slider->objectName())).toInt();
            slider->setValue(indexbaud );
        }
    }

}
//显示非自定义模式
void DequalizerDialog::showCurMode(int *Allbauds)
{
    Q_D(DequalizerDialog);
    if (Allbauds == nullptr) {
        return;
    }
    int i = 0;
    for (DSlider *slider : findChildren<DSlider *>()) {
        if (d->mswitchBtn->isChecked()) {
            slider->setEnabled(true);
        }
        slider->setValue(Allbauds[i]);
        slider->setToolTip(QString::number(slider->value()) + "dB");
        i++;
    }
    d->saveBtn->setEnabled(false);
}

//开关
void DequalizerDialog::checkedChanged(bool checked)
{
    Q_D(DequalizerDialog);
    for (DSlider *slider : findChildren<DSlider *>()) {
        slider->setEnabled(checked);
    }
    d->mswitchLabel->setText("关");
    if (checked) {
        d->mswitchLabel->setText("开");
        //flat;
        showCurMode(d->flat_bauds);
        d->mcombox->setCurrentIndex(1);
        Q_EMIT setEqualizerIndex(1);
        d->settings->setOption("equalizer.all.curEffect", 1);
    };
    d->mcombox->setEnabled(checked);
    d->saveBtn->setEnabled(false);
    Q_EMIT setEqualizerEnable(checked);
    d->settings->setOption("equalizer.all.switch", checked);
}

//恢复默认
void DequalizerDialog::setDefaultClicked()
{
    Q_D(DequalizerDialog);
    if (d->mswitchBtn->isChecked()) {
        showCurMode(d->flat_bauds);
        d->mswitchBtn->setChecked(false);
        d->mcombox->setCurrentIndex(1);
        Q_EMIT setEqualizerEnable(false);
        d->settings->setOption("equalizer.all.curEffect", 1);
    }
}
