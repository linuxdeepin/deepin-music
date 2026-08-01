// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
            EffectLine {
                id: around
                width: parent.width
                height: parent.height
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
            EffectLightWave {
                id: around
                width: parent.width
                height: parent.height
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
            EffectSun {
                id: around
                width: parent.width
                height: parent.height

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
            EffectWaterWave {
                id: around
                width: parent.width
                height: parent.height
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
            EffectParticle {
                id: around
                width: parent.width
                height: parent.height
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
            Rectangle {
                id: around
                width: parent.width
                height: parent.height
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
