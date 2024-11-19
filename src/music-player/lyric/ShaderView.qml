// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.0
import "effectpublic"
import Qt.labs.platform 1.0


Item {
    property int  currentItem: -1
    signal sigShaderStatusChange(int shaderStatus)
    property var stackView: stack
    property var taskMap: {
        0: effectLineView,
        1: effectLightWaveView,
        2: effectSunView,
        3: effectWaterWaveView,
        4: effectLightWaveView,
        // 4: effectParticleView,
        5: commView,
    }
    Row {
        id: grid
        Item {
            id: element1
            width: leftArea.width
            height: leftArea.height
            visible: true

            StackView {
                id: stack
                width: parent.width;
                height: parent.height;


                Component.onCompleted: {
                    stack.push(commView);
                }
            }
        }
    }
    Component {
        id: effectLineView;
        Item {
            anchors.fill: parent
            EffectLine {
                id: around
                width: parent.width
                height: parent.height
                anchors.centerIn: parent
                Circular_img {
                    id:circular_img
                    anchors.centerIn: around
                    background_width:around.width*1.8/3
                    background_height:around.height*1.8/3
                }
            }
        }
    }
    Component {
        id: effectLightWaveView;
        Item {
            anchors.fill: parent

            EffectLightWave {
                id: around
                width: parent.width
                height: parent.height
                anchors.centerIn: parent
                Circular_img {
                    id:circular_img
                    anchors.centerIn: around
                    background_width:around.width*1.8/3
                    background_height:around.height*1.8/3
                }
            }
        }
    }
    Component {
        id: effectSunView;
        Item {
            anchors.fill: parent
            EffectSun {
                id: around
                width: parent.width
                height: parent.height
                anchors.centerIn: parent

                Circular_img {
                    id:circular_img
                    anchors.centerIn: around
                    background_width:around.width*1.8/3
                    background_height:around.height*1.8/3
                }
            }
        }
    }
    Component {
        id: effectWaterWaveView;
        Item {
            anchors.fill: parent
            EffectWaterWave {
                id: around
                width: parent.width
                height: parent.height
                anchors.centerIn: parent
                Circular_img {
                    id:circular_img
                    anchors.centerIn: around
                    background_width:around.width*1.8/3
                    background_height:around.height*1.8/3
                }
            }
        }
    }
    Component {
        id: effectParticleView;
        Item {
            anchors.fill: parent
            EffectParticle {
                id: around
                width: parent.width
                height: parent.height
                anchors.centerIn: parent
                Circular_img {
                    id:circular_img
                    anchors.centerIn: around
                    background_width:around.width*1.8/3
                    background_height:around.height*1.8/3
                }
            }
        }
    }
    Component {
        id: commView
        Item {
            id: commItem
            anchors.fill: parent
            Rectangle {
                id: around
                width: parent.width
                height: parent.height
                anchors.centerIn: parent
                color: "transparent"
                Circular_img {
                    id:circular_img
                    anchors.centerIn: around
                    background_width:around.width*1.8/3
                    background_height:around.height*1.8/3
                }
            }
        }
    }
}
