import QtQuick 2.0
import QtQuick.Window 2.2
import DMusic 1.0

Window {
    id: miniWindow

    property var miniWindowController: miniWindowController

    property var bgImage: bgImage

    color: "#282F3F"

    flags: Qt.FramelessWindowHint | Qt.Dialog | Qt.WindowStaysOnTopHint

    BorderImage {
        id: bgImage
        objectName: 'bgImage'

        width: miniWindow.width
        height: miniWindow.height

        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    MouseArea {

        width: miniWindow.width
        height: miniWindow.height

        property int dragStartX
        property int dragStartY
        property int windowLastX
        property int windowLastY

        propagateComposedEvents: true

        onPressed: { 
            var pos = WindowManageWorker.cursorPos
            
            windowLastX = miniWindow.x
            windowLastY = miniWindow.y
            dragStartX = pos.x
            dragStartY = pos.y
            print('DMoveable mouse')
        }
        onPositionChanged: {
            if (pressed) {
                var pos = WindowManageWorker.cursorPos
                miniWindow.x = (windowLastX + pos.x - dragStartX)
                miniWindow.y = (windowLastY + pos.y - dragStartY)
                windowLastX = miniWindow.x
                windowLastY = miniWindow.y
                dragStartX = pos.x
                dragStartY = pos.y
            }
        }

        onDoubleClicked : {
            WindowManageWorker.showNormal();
        }
    }

    Rectangle {
        id: playBottomBar

        property var slider: slider
        property var playControl: playControl
        property var preButton: preButton
        property var playButton: playButton
        property var nextButton: nextButton

        width: miniWindow.width
        height: miniWindow.height
        color: "transparent"

        Column {
            Rectangle {
                id: playControl

                property int iconsWidth: 40
                property int iconsHeight: iconsWidth
                property bool playing: false

                width: playBottomBar.width
                height: playBottomBar.height - slider.height
                color: "transparent"

                signal played(bool isPlaying)

                Row {
                    anchors.centerIn: parent

                    DPreButton{
                        id: preButton
                        width: playControl.iconsWidth
                        height: playControl.iconsHeight
                    }

                    DPlayButton{
                        id: playButton
                        playing: playControl.playing

                        width: playControl.iconsWidth
                        height: playControl.iconsHeight
                    }

                    DNextButton{
                        id: nextButton
                        width: playControl.iconsWidth
                        height: playControl.iconsHeight
                    }
                }
            }

            SimpleSlider {
                id: slider
                objectName: 'simpleSlider'

                progressBarColor: '#0AB9E2'
                width: playBottomBar.width
                height: 6
            }
        }

    }

    MiniWindowController {
        id: miniWindowController
        miniWindow: miniWindow
        bgImage: bgImage
        playBottomBar: playBottomBar
    }
}