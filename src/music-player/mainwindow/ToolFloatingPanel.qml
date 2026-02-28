// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

D.Control {
    id: control

    padding: DS.Style.floatingPanel.radius / 2

    property D.Palette backgroundColor: DS.Style.floatingPanel.background
    property D.Palette dropShadowColor: DS.Style.floatingPanel.dropShadow
    property D.Palette outsideBorderColor: DS.Style.floatingPanel.outsideBorder
    property D.Palette insideBorderColor: DS.Style.floatingPanel.insideBorder
    // corner radius
    property int radius: DS.Style.floatingPanel.radius
    // blur radius
    property int blurRadius: DS.Style.floatingPanel.radius
    // blur blurMultiplier
    property real blurMultiplier: 0.0

    background: D.InWindowBlur {
        implicitWidth: DS.Style.floatingPanel.width
        implicitHeight: DS.Style.floatingPanel.height
        radius: blurRadius
        offscreen: true

        D.ItemViewport {
            anchors.fill: parent
            fixed: true
            sourceItem: parent
            radius: control.radius
            hideSource: false
        }

        D.BoxShadow {
            anchors.fill: backgroundRect
            shadowOffsetX: 0
            shadowOffsetY: 6
            shadowColor: control.D.ColorSelector.dropShadowColor
            shadowBlur: 20
            cornerRadius: backgroundRect.radius
            spread: 0
            hollow: true
        }

        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            radius: control.radius
            color: control.D.ColorSelector.backgroundColor
        }

        Loader {
            anchors.fill: backgroundRect
            active: control.D.ColorSelector.controlTheme === D.ApplicationHelper.DarkType
            sourceComponent: D.InsideBoxBorder {
                radius: backgroundRect.radius
                color: control.D.ColorSelector.insideBorderColor
                borderWidth: DS.Style.control.borderWidth
            }
        }

        D.OutsideBoxBorder {
            anchors.fill: backgroundRect
            radius: backgroundRect.radius
            color: control.D.ColorSelector.outsideBorderColor
            borderWidth: DS.Style.control.borderWidth
        }
    }
}
