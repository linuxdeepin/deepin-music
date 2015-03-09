import QtQuick 2.4
import DMusic 1.0

DIconButton{
    id: playButton

    property bool playing: false

    normal_image: playButton.playing ?  '../../skin/icons/dark/appbar.control.pause.png' : '../../skin/icons/dark/appbar.control.play.png'
    hover_image: playButton.playing ? '../../skin/icons/dark/appbar.control.pause.png': '../../skin/icons/dark/appbar.control.play.png'
    pressed_image: playButton.playing ? '../../skin/icons/dark/appbar.control.pause.png' : '../../skin/icons/light/appbar.control.play.png'
    disabled_image: playButton.playing ? '../../skin/icons/dark/appbar.control.pause.png' : '../../skin/icons/dark/appbar.control.play.png'

    // tootip: DToolTip {
    //     id: tooltip1
    //     width: 200
    //     text: "Enter the text here."
    // }
}