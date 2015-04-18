import QtQuick 2.3

Item {
    property var miniWindow
    property var constants
    property var bgImage
    property var playBottomBar


   function resetSkin() {
        playBottomBar.color = "transparent";
        bgImage.source = constants.defaultBackgroundImage;
    }

    function setSkinByImage(url) {
        if(url){
            bgImage.source = url;
        }else{
            resetSkin();
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
}
