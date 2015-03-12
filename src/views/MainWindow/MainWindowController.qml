import QtQuick 2.4

Item {
    property var mainWindow
    property var bgImage
	property var titleBar
	property var leftSideBar
	property var webEngineViewPage
    property var playBottomBar
    property var dSimpleWindow


    function resetSkin() {
        playBottomBar.color = "#282F3F"
        bgImage.source = ''
    }


    function setSkinByImage(url) {
        if (url){
            playBottomBar.color = "transparent"
            bgImage.source = url
        }else{
            url = "../../skin/images/bg2.jpg"
            playBottomBar.color = "#282F3F"
            bgImage.source = ''
        }
    }

    Connections {
        target: leftSideBar
        onSwicthViewByID: {
            var index =  mainWindow.views.indexOf(viewID);
            stackViews.setCurrentIndex(index);
        }
    }

    Connections {
        target: webEngineViewPage
        onPlayMusicByID: Web360ApiWorker.getMusicURLByID(musicID)
    }

}
