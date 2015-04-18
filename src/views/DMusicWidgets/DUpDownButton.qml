import QtQuick 2.3
import DMusic 1.0

DIconButton{
    id: root
    property bool switchflag: false

    normal_image: root.switchflag ?  '../../skin/icons/light/appbar.chevron.up.png' : '../../skin/icons/light/appbar.chevron.down.png'
    hover_image: root.switchflag ? '../../skin/icons/light/appbar.chevron.up.png': '../../skin/icons/light/appbar.chevron.down.png'
    pressed_image: root.switchflag ? '../../skin/icons/light/appbar.chevron.up.png' : '../../skin/icons/light/appbar.chevron.down.png'
    disabled_image: root.switchflag ? '../../skin/icons/light/appbar.chevron.up.png' : '../../skin/icons/light/appbar.chevron.down.png'

}