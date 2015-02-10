import QtQuick 2.0
import QtQuick.Window 2.1
import DMusic 1.0
Item {
    id: root
   
    width: 960
    height: 660

    objectName: "mainLoader"

    DSplash{
        id: splash
        showDuration: 0
        pauseDuration: 0
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
            mainLoader.source="mainmusic.qml"
        }
    }

    Loader{
        anchors.fill: parent
        focus: true
        id: mainLoader
    }
}
