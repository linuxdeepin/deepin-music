import QtQuick 2.4

Item {
    property var simpleWindow
    property var bgImage
	property var titleBar
    property var switchButton
    property var stackViews
    property var playlistPage
    property var lrcPage
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

    Connections {
        target: switchButton
        onClicked: {
            switchButton.switchflag = !switchButton.switchflag;
            var index =  1 - stackViews.currentIndex
            stackViews.setCurrentIndex(index);
        }
    }
}
