import QtQuick 2.0

Item {
    property var simpleWindow
    property var bgImage
	property var titleBar
	property var playBottomBar

    function resetSkin() {
        playBottomBar.color = "#282F3F"
        bgImage.source = ''
    }


    function setSkinByImage(url) {
        if (url === undefined){
            url = "../../skin/images/bg2.jpg"
        }
        playBottomBar.color = "transparent"
        bgImage.source = url
    }
}
