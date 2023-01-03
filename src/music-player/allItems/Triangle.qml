// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11

Canvas {
    property color fillColor: "#474747"

    id: triangleCanvas
    width: parent.width; height: parent.height
    contextType: "2d"

    onPaint: {
        context.lineWidth = 0
        context.fillStyle = fillColor
        context.beginPath();
        context.moveTo(0, height)
        context.lineTo(width, height);
        context.lineTo(width / 2, 0);
        context.closePath();
        context.fill()
    }
    onFillColorChanged: {
        triangleCanvas.requestPaint()
    }
}
