import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: lockButton
    property bool locked: false
    normal_opacity: 1
    color: "transparent"
    normal_image: lockButton.locked ?  '../../../skin/svg/lrc.desktop.lock.svg' :  '../../../skin/svg/lrc.desktop.unlock.svg'
}