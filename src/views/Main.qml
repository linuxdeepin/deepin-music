import QtQuick 2.0
import QtQuick.Window 2.1
import DMusic 1.0
Item {
    id: root
    objectName: "root"

    property var dragPostion

    width: 960
    height: 660

    

    DSplash{
        id: splash
        showDuration: 0
        pauseDuration: 2000
        hideDuration: 0
        anchors.fill: parent
        Text{
            anchors.centerIn: splash
            text: 'Welcome, this is Splash!'
            font.family: "微软雅黑"
            font.pointSize: 20
            color: "white"
        }

        onFinished:{
            mainLoader.opacity = 1
        }
    }

    MainMusic{
        id: mainLoader
        objectName: "MainMusic"

        anchors.fill: parent
        opacity: 0
        focus: true
    }
}
