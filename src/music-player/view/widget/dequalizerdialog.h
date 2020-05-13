#ifndef DEQUALIZERDIALOG_H
#define DEQUALIZERDIALOG_H
#include <DAbstractDialog>
#include "../core/musicsettings.h"
#include "vlc/Equalizer.h"

class DequalizerDialogPrivate;
class DequalizerDialog: public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT
public:
    explicit DequalizerDialog(QWidget *parent = Q_NULLPTR);
    ~DequalizerDialog();
    void readConfig(MusicSettings *settings);
    void writeConfig(MusicSettings *settings);
    void enabledUI(bool flag);
    void initConnection();
    void customMode();
    void setMediaPlayer(VlcMediaPlayer *mediaPlayer);

signals:
    void getCurIndex(int index);

public Q_SLOTS:
    void updateSettings();
//    void updateSettings(DTK_CORE_NAMESPACE::DSettings *settings);
//    void updateSettings(const QByteArray &translateContext, DTK_CORE_NAMESPACE::DSettings *settings);
    void applyChangesForBand(int value);
    void applySelectedPreset();
    void checkedChanged(bool checked);

private:
    QScopedPointer<DequalizerDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DequalizerDialog)
};

#endif // DEQUALIZERDIALOG_H
