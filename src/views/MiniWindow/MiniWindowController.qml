import QtQuick 2.4

Item {
    property var miniWindow
    property var bgImage
    property var playBottomBar


    function resetSkin() {
        playBottomBar.color = "#282F3F"
        bgImage.source = ''
    }


    function setSkinByImage(url) {
        if (url === undefined){
            url = "http://p1.qhimg.com/dr/100_100_100/t018ac6dcdcdfced8fd.jpg"
        }
        playBottomBar.color = "transparent"
        bgImage.source = url
    }
}
