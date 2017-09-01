#include "combobox.h"

ComboBox::ComboBox(QWidget *parent) :
    QComboBox(parent)
{

}

void ComboBox::wheelEvent(QWheelEvent *e)
{
    if (hasFocus()) {
        QComboBox::wheelEvent(e);
    }
}
