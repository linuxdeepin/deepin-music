// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0

FloatingPanel {
    property int volume: 0
    property bool isMute: false

    id: control
    width: 60
    height: 250
    visible: false

    MouseArea {
        anchors.fill: parent

        onWheel: {
            var step = wheel.angleDelta.y / 8 / 1.5
            contentSlider.value += step

            if (contentSlider.value < 0)
                contentSlider.value = 0

            if (contentSlider.value > 100)
                contentSlider.value = 100
        }
    }
    ColumnLayout {
        height: 200
        spacing: 10
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        Label {
            text: {
                return Number(contentSlider.value).toFixed(0) + "%"
            }
            Layout.alignment: Qt.AlignHCenter
        }
        Slider {
            id: contentSlider
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            highlightedPassedGroove: true
            handleType: Slider.HandleType.NoArrowVertical
            leftPadding: 15

            from: 0
            to: 100
            value: volume
            orientation: Qt.Vertical

            onValueChanged: {
                //volume = value
                Presenter.setVolume(value)
            }
        }
        ActionButton {
            icon.name: isMute ? "toolbar_volume-" : "toolbar_volume+"
            icon.width: 36
            icon.height: 36
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                Presenter.setMute(!isMute)
            }
        }
    }

    function onMuteChanged(mute) {
        isMute = mute
    }
    function onVolumeChanged(value) {
        volume = value
    }

    Component.onCompleted: {
        volume = Presenter.getVolume();
        isMute = Presenter.getMute()

        Presenter.muteChanged.connect(onMuteChanged)
        Presenter.volumeChanged.connect(onVolumeChanged)
    }
}
