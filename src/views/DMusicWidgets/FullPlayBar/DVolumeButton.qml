import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: root
    property bool switchflag: true
    normal_opacity: 0.8
    normal_image: root.switchflag ? '../../../skin/svg/full.volume.3.svg': '../../../skin/svg/full.volume.mute.svg'
}