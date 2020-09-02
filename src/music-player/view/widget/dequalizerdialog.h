#ifndef DEQUALIZERDIALOG_H
#define DEQUALIZERDIALOG_H
#include <DAbstractDialog>
#include <QToolTip>
#include "../core/musicsettings.h"
#include "vlc/Equalizer.h"

class DequalizerDialogPrivate;
class DequalizerDialog: public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT
public:
    explicit DequalizerDialog(QWidget *parent = Q_NULLPTR);
    ~DequalizerDialog();

    void initConnection();
    void showCustom();
    void showCurMode(int *Allbauds);

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
    QScopedPointer<DequalizerDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DequalizerDialog)
};

#endif // DEQUALIZERDIALOG_H
