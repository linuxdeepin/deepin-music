import QtQuick 2.0

Item {

	property var titleBar
	property var leftSideBar
	property var webEngineViewPage
	property var player

	Connections {
        target: titleBar
        onShowMinimized: MainWindow.showMinimized()
        onMenuShowed: MenuWorker.showSettingMenu()
    }

    Connections {
        target: player
        onMediaObjectChanged: {
            if (player.mediaObject)
                player.mediaObject.notifyInterval = 50;
        }
    }

    Connections {
        target: leftSideBar
        onSwicthViewByID: {
            if (viewID == 'WebMusic360Page'){
                webEngineViewPage.url = "http://10.0.0.153:8093/";
            }
            else if (viewID == 'MusicManagerPage'){
                webEngineViewPage.url = MusicManageWorker.artistUrl
            }
            else if (viewID == 'PlayListPage'){
                webEngineViewPage.url = "https://www.baidu.com";
            }
            else if (viewID == 'DownloadPage'){
                webEngineViewPage.url = "http://www.oschina.net/";
            }else{
                console.log('--------------No Page--------------');
            }
        }
    }
}
