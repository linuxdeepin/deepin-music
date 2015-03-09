import QtQuick 2.4
import DMusic 1.0

DIconButton{
    id: root
    property bool switchflag: true
    normal_image: root.switchflag ? '../../skin/icons/dark/appbar.sound.3.png': '../../skin/icons/dark/appbar.sound.mute.png'
    hover_image: root.switchflag ? '../../skin/icons/dark/appbar.sound.3.png': '../../skin/icons/dark/appbar.sound.mute.png'
    pressed_image: root.switchflag ? '../../skin/icons/light/appbar.sound.3.png' : '../../skin/icons/dark/appbar.sound.mute.png'
    disabled_image: root.switchflag ? '../../skin/icons/dark/appbar.sound.3.png': '../../skin/icons/dark/appbar.sound.mute.png'
}