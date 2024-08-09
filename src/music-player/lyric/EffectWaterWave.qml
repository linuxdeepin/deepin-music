// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import audio.image 1.0
import "effectpublic"

CusShaderToy {
    id: r

    //step 6: around
    property real wave
    property real speed
    property real ringWidth
    property real rotationSpeed
    property real holeSmooth
    property real holeSize
    property real colorSaturation
    property real colorValue
    pixelShader: "qrc:/lyric/qsbfile/EffectWaterWave.qsb"
    iChannel1: Image {
        source: "qrc:/dsg/img/bk.png"
    }
    iChannel0: View_data {
        id : _image
        visible: true
        width: 1024
        height: 1
        anchors.centerIn: parent
        presenter: Presenter
    }
    iColor: Qt.vector3d(Presenter.getMainColorByKmeans().r, Presenter.getMainColorByKmeans().g, Presenter.getMainColorByKmeans().b)
}
