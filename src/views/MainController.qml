import QtQuick 2.4

Item {

    property var rootWindow
    property var mainWindow
    property var simpleWindowLoader
    property var miniWindow
    property var constants

    function initConnect() {
        WindowManageWorker.mainWindowShowed.connect(showMainWindow);
        WindowManageWorker.simpleWindowShowed.connect(showSimpleWindow);
        WindowManageWorker.miniWindowShowed.connect(showMiniWindow);
        Qt.globalPos = WindowManageWorker.cursorPos;
        MenuWorker.miniTrigger.connect(showMiniWindow);
    }

    function showMainWindow() {
        MainWindow.show();
        rootWindow.width = constants.mainWindowWidth;
        rootWindow.height = constants.mainWindowHeight;
        mainWindow.visible = true;
        simpleWindowLoader.source = ''
        simpleWindowLoader.focus = false;

        miniWindow.visible = false;

        WindowManageWorker.windowMode = 'Full'
        WindowManageWorker.lastWindowMode = 'Full'
    }

    function showSimpleWindow() {
        MainWindow.show();
        rootWindow.width = constants.simpleWindowWidth;
        rootWindow.height = constants.simpleWindowHeight;
        mainWindow.visible = false;
        simpleWindowLoader.source = './SimpleWindow/SimpleWindow.qml';
        simpleWindowLoader.focus = true;

        miniWindow.visible = false;
        WindowManageWorker.windowMode = 'Simple'
        WindowManageWorker.lastWindowMode = 'Simple'
    }

    function showMiniWindow() {
        miniWindow.visible = true;
        MainWindow.hide();

        WindowManageWorker.windowMode = 'Mini'
    }

    function closeAll() {
        Qt.quit();
    }

    function destoryTermporyWindow(){
        mainWindow.termporyLoader.source = ''
        mainWindow.focus = true;
    }

	Connections {
        target: mainWindow.titleBar

        onSimpleWindowShowed: {
            showSimpleWindow();
        }
        
        onMenuShowed: {
            MenuWorker.showSettingMenu();
        }

        onShowMinimized: {
            MainWindow.showMinimized()
        }

        onClosed: {
            closeAll();
        }
    }

    Connections {
        target: mainWindow.playBottomBar.playlistButton
        onClicked:{
            if(mainWindow.termporyLoader.source == ''){
                mainWindow.termporyLoader.source = './TermporyWindow/TermporyWindow.qml'
                mainWindow.termporyLoader.focus = true;
                mainWindow.focus = false;
            }else{
                destoryTermporyWindow();
            }
        } 
    }

    Connections {
        target: mainWindow.termporyLoader
        onLoaded:{
            var termporyWindow = mainWindow.termporyLoader.item;
            if (termporyWindow){
                var closeButton = termporyWindow.closeButton;
                closeButton.clicked.connect(destoryTermporyWindow);
            }
        }
    }

    Connections {
        target: miniWindow

        onExpandNoraml : {
            WindowManageWorker.showNormal();
        }

        onClosed:{
            closeAll();
        }
    }

    Component.onCompleted: {
        initConnect();
    }
}
