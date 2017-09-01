/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include <QFrame>

class ButtonGroupPrivate;
class ButtonGroup : public QFrame
{
    Q_OBJECT
public:
    explicit ButtonGroup(QWidget *parent = 0);
    ~ButtonGroup();

Q_SIGNALS:
    void buttonChecked(int id);

public Q_SLOTS:
    void setCheckedButton(int id);
    void setButtons(const QStringList &texts);

private:
    QScopedPointer<ButtonGroupPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ButtonGroup)
};

