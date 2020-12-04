#ifndef DEQUALIZERDIALOG_H
#define DEQUALIZERDIALOG_H

#include <DAbstractDialog>

#include <QToolTip>
#include <DTitlebar>
#include <DSwitchButton>
#include <DTabWidget>
#include <DComboBox>
#include <DSlider>
#include <DBlurEffectWidget>
#include <DFloatingMessage>
#include <DPushButton>
#include <DTitlebar>
#include <DLabel>

#include "../core/musicsettings.h"
#include "vlc/Equalizer.h"


DWIDGET_USE_NAMESPACE


class CustomTabStyle;

class DequalizerDialog: public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT
public:
    explicit DequalizerDialog(QWidget *parent = Q_NULLPTR);
    ~DequalizerDialog();

    void initConnection();
    void showCustom();
    void showCurMode(int *Allbauds);

    void readConfig();
    void initUI();
    void SliderOneEntry(DSlider *slider, int value);

signals:
    void setEqualizerEnable(bool enabled);
    void setEqualizerIndex(int curIndex);
    void setEqualizerpre(int val);
    void setEqualizerbauds(int index, int val);

public Q_SLOTS:
    void checkedChanged(bool checked);
    void setDefaultClicked();

private:
    void selectSlider(QObject *obj, QString val);
    bool eventFilter(QObject *obj, QEvent *event);

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
    DFloatingMessage *saveMessage     = nullptr;
    CustomTabStyle  *tabStyle        = nullptr;

    QStringList    effect_type        =  { DequalizerDialog::tr("Custom")
                                           , DequalizerDialog::tr("Monophony")
                                           , DequalizerDialog::tr("Classical")
                                           , DequalizerDialog::tr("Club")
                                           , DequalizerDialog::tr("Dance")
                                           , DequalizerDialog::tr("Full Bass")
                                           , DequalizerDialog::tr("Full Bass and Treble")
                                           , DequalizerDialog::tr("Full Treble")
                                           , DequalizerDialog::tr("Headphones")
                                           , DequalizerDialog::tr("Hall")
                                           , DequalizerDialog::tr("Live")
                                           , DequalizerDialog::tr("Party")
                                           , DequalizerDialog::tr("Pop")
                                           , DequalizerDialog::tr("Reggae")
                                           , DequalizerDialog::tr("Rock")
                                           , DequalizerDialog::tr("Ska")
                                           , DequalizerDialog::tr("Soft")
                                           , DequalizerDialog::tr("Soft Rock")
                                           , DequalizerDialog::tr("Techno")
                                         };
    QList<int> BaudList;

    MusicSettings   *settings         = nullptr;


    bool    switchflag               = false;
    bool    changeflag                = false;
    int     curEffect                 = 0;

    int flat_bauds[11] = {12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int Classical_bauds[11] = {12, 0, 0, 0, 0, 0, 0, -7, -7, -7, -9};
    int Club_bauds[11] = {6, 0, 0, 8, 5, 5, 5, 3, 0, 0, 0};
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
};

#endif // DEQUALIZERDIALOG_H
