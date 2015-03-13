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
        playBottomBar.color = "transparent";
        bgImage.source = "../../skin/images/bg2.jpg";
    }

    function setSkinByImage(url) {
        if(url){
            bgImage.source = url;
        }else{
            url = "../../skin/images/bg2.jpg";
        }
    }

    Connections {
        target: bgImage
        onProgressChanged:{
            if (progress == 1){
                playBottomBar.color = 'transparent'
            }
        }
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
