import QtQuick 2.0

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
        if (url === undefined){
            url = "../../skin/images/bg2.jpg"
        }
        playBottomBar.color = "transparent"
        bgImage.source = url
    }

    // Connections {
    //     target: leftSideBar
    //     onSwicthViewByID: {
    //         if (viewID == 'WebMusic360Page'){
    //             webEngineViewPage.url = "http://10.0.0.153:8093/";
    //         }
    //         else if (viewID == 'MusicManagerPage'){
    //             webEngineViewPage.url = MusicManageWorker.artistUrl
    //         }
    //         else if (viewID == 'PlayListPage'){
    //             webEngineViewPage.url = "https://www.baidu.com";
    //         }
    //         else if (viewID == 'DownloadPage'){
    //             webEngineViewPage.url = "file:///tmp/90.html";
    //         }else{
    //             console.log('--------------No Page--------------');
    //         }
    //     }
    // }


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
