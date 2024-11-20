// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
// TODO: 兼容使用，BackdropBlitter 正常后替换
import QtQuick
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
D.Control {
    id: control
    property D.Palette backgroundColor: DS.Style.floatingPanel.background
    property D.Palette backgroundNoBlurColor: DS.Style.floatingPanel.backgroundNoBlur
    // blur blurMultiplier
    property real blurMultiplier: 0.0
    // blur radius
    property int blurRadius: 64
    property D.Palette dropShadowColor: DS.Style.floatingPanel.dropShadow
    property D.Palette insideBorderColor: DS.Style.floatingPanel.insideBorder
    property D.Palette outsideBorderColor: DS.Style.floatingPanel.outsideBorder
    // corner radius
    property int radius: DS.Style.floatingPanel.radius
    padding: DS.Style.floatingPanel.radius / 2
    background: D.InWindowBlurImpl {
        id: blur
        implicitHeight: DS.Style.floatingPanel.height
        implicitWidth: DS.Style.floatingPanel.width
        // multiplier: blurMultiplier
        offscreen: true
        radius: blurRadius
        D.ItemViewport {
            anchors.fill: parent
            fixed: true
            hideSource: false
            radius: control.radius
            sourceItem: parent
        }
        Loader {
            active: Window.window && Window.window.color.a < 1
            anchors.fill: parent
            sourceComponent: D.ItemViewport {
                anchors.fill: parent
                compositionMode: DTK.CompositionMode.Source
                fixed: true
                hideSource: false
                radius: control.radius
                sourceItem: blur.content
            }
        }
        Loader {
            active: control.dropShadowColor
            anchors.fill: backgroundRect
            sourceComponent: D.BoxShadow {
                cornerRadius: backgroundRect.radius
                hollow: true
                shadowBlur: 20
                shadowColor: control.D.ColorSelector.dropShadowColor
                shadowOffsetX: 0
                shadowOffsetY: 6
                spread: 0
            }
        }
        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            color: blur.valid ? control.D.ColorSelector.backgroundColor : control.D.ColorSelector.backgroundNoBlurColor
            radius: control.radius
        }
        Loader {
            active: control.insideBorderColor && control.D.ColorSelector.controlTheme === D.ApplicationHelper.DarkType
            anchors.fill: backgroundRect
            sourceComponent: D.InsideBoxBorder {
                borderWidth: DS.Style.control.borderWidth
                color: control.D.ColorSelector.insideBorderColor
                radius: backgroundRect.radius
            }
        }
        Loader {
            active: control.outsideBorderColor
            anchors.fill: backgroundRect
            sourceComponent: D.OutsideBoxBorder {
                borderWidth: DS.Style.control.borderWidth
                color: control.D.ColorSelector.outsideBorderColor
                radius: backgroundRect.radius
            }
        }
    }
}