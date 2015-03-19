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
            bgImage.source = url
        }else{
            resetSkin();
        }
    }

    Connections {
        target: bgImage
        onProgressChanged:{
            if (progress == 1){
                playBottomBar.color = "transparent"
            }
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
        onPlayMusicById: Web360ApiWorker.playMusicById(musicId)
        onPlayMusicByIds: Web360ApiWorker.playMusicByIds(musicIds)
    }
}
