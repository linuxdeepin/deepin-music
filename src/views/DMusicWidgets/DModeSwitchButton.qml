import QtQuick 2.3
import DMusic 1.0

DIconButton{
	id: root
	property var windowFlag: true
    normal_image: windowFlag?'../../skin/titlebar/mode_switch_1.png':'../../skin/titlebar/mode_switch_2.png'
    hover_image: windowFlag?'../../skin/titlebar/mode_switch_1.png': '../../skin/titlebar/mode_switch_2.png'
    pressed_image: windowFlag?'../../skin/titlebar/dark_mode_switch_1.png':'../../skin/titlebar/dark_mode_switch_2.png'
    disabled_image: windowFlag?'../../skin/titlebar/mode_switch_1.png':'../../skin/titlebar/mode_switch_2.png'
}
