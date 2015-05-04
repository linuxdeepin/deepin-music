import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: root

    property bool downloading
    property var start_image: '../../../skin/svg/download.start.svg'
    property var pause_image: '../../../skin/svg/download.pause.svg'


    normal_opacity: 1
    normal_image: ''
    hovered_image: downloading? pause_image: start_image
    pressed_image: downloading? start_image: pause_image

}