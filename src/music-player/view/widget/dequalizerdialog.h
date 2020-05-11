#ifndef DEQUALIZERDIALOG_H
#define DEQUALIZERDIALOG_H
#include <DAbstractDialog>
#include "../core/musicsettings.h"


class DequalizerDialogPrivate;
class DequalizerDialog: public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT
public:
    explicit DequalizerDialog(QWidget *parent = Q_NULLPTR);
    ~DequalizerDialog();

signals:
    void entered();

public Q_SLOTS:
//    void updateSettings(DTK_CORE_NAMESPACE::DSettings *settings);
//    void updateSettings(const QByteArray &translateContext, DTK_CORE_NAMESPACE::DSettings *settings);

private:
    QScopedPointer<DequalizerDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DequalizerDialog)

};

#endif // DEQUALIZERDIALOG_H
