import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: root
    property bool stared: false
    normal_opacity: 0.8
    color: "transparent"
    normal_image: root.stared ?  '../../../skin/svg/favorite.on.svg' :  '../../../skin/svg/favorite.off.svg'
}