import QtQuick 2.0
import DMusic 1.0

DIconButton{
	id: root
    property bool switchflag: false

    normal_image: root.switchflag ?  '../../skin/icons/dark/appbar.arrow.left.right.png' : '../../skin/icons/dark/appbar.arrow.left.right.png'
    hover_image: root.switchflag ? '../../skin/icons/dark/appbar.arrow.left.right.png': '../../skin/icons/dark/appbar.arrow.left.right.png'
    pressed_image: root.switchflag ? '../../skin/icons/dark/appbar.arrow.left.right.png' : '../../skin/icons/light/appbar.arrow.left.right.png'
    disabled_image: root.switchflag ? '../../skin/icons/dark/appbar.arrow.left.right.png' : '../../skin/icons/dark/appbar.arrow.left.right.png'

}