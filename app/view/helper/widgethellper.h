/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef WIDGETHELLPER_H
#define WIDGETHELLPER_H

#include <QPixmap>
#include <QImage>

namespace WidgetHellper
{
    QPixmap blurImage(const QImage &image, int radius);
}

#endif // WIDGETHELLPER_H
