import QtQuick 2.3
import DMusic 1.0

DSvgButton{
    id: favoriteButton
    property bool isFavorite: false
    normal_opacity: 0.8
    color: "transparent"
    normal_image: favoriteButton.isFavorite ? '../../../skin/svg/favorite.on.svg' :  '../../../skin/svg/favorite.off.svg'
}