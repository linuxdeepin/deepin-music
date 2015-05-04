import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: root
    property bool switchflag: true
    normal_opacity: 0.8
    normal_image: root.switchflag ? '../../../skin/svg/simple.volume.3.svg': '../../../skin/svg/simple.volume.mute.svg'
}