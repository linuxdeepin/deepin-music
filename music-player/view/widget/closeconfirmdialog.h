#pragma once

#include <QScopedPointer>
#include <dabstractdialog.h>
#include <ddialog.h>

class CloseConfirmDialogPrivate;
class CloseConfirmDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    explicit CloseConfirmDialog(QWidget *parent = 0);
    ~CloseConfirmDialog();

Q_SIGNALS:
    void quitAction(bool ask, int quitAction);

public Q_SLOTS:

private:
    QScopedPointer<CloseConfirmDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), CloseConfirmDialog)
};

