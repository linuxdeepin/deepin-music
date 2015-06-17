import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: lockButton
    property bool locked: false
    normal_opacity: 0.8
    color: "transparent"
    normal_image: lockButton.locked ?  '../../../skin/svg/lrc.lock.svg' :  '../../../skin/svg/lrc.unlock.svg'
}