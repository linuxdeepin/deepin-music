// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.4
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Rectangle {
    property url imageUrl
    id: titleBackground
    color: "transparent"
    Canvas {
        id: canvasImage
        width: titleBackground.width; height: titleBackground.height
        smooth: true;
        antialiasing: true
        opacity: 0.0
        onImageLoaded: requestPaint()
        Component.onCompleted: {
            canvasImage.loadImage(imageUrl);
        }
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0,0,canvasImage.width,canvasImage.height)
            ctx.drawImage(imageUrl, 0, 0, canvasImage.width, 224);
            var ImageData = ctx.getImageData(0,0,canvasImage.width, 224)
            ctx.drawImage(ImageData, 0, 224/3, canvasImage.width,224/3, 0, 0,canvasImage.width, canvasImage.height);
            var liner = ctx.createLinearGradient(0, 0, 0, 224);
            liner.addColorStop(0, Qt.rgba(255,255,255,0.0))
            liner.addColorStop(0.5, Qt.rgba(255,255,255,0.5))
            liner.addColorStop(0.95, Qt.rgba(255,255,255,0.95))
            ctx.fillStyle = liner
            ctx.fillRect(0,0,canvasImage.width,canvasImage.height);
            ctx.strokeRect(0,0,canvasImage.width,canvasImage.height);
        }
    }
    FastBlur {
        anchors.fill: canvasImage
        source: canvasImage
        radius: 64
        transparentBorder: true
    }
}
