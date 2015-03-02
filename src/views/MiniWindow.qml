import QtQuick 2.0
import QtQuick.Window 2.2
import DMusic 1.0
import "./dmusicwidgets"

Window {
    id: miniWindow

    property var miniWindowController: miniWindowController

    property var bgImage: bgImage

    color: "#282F3F"

    flags: Qt.FramelessWindowHint | Qt.Dialog | Qt.WindowStaysOnTopHint


    signal closed()
    signal expandNoraml()
    signal preMusic()
    signal lrcShow()
    signal nextMusic()

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
    }

    Rectangle {
        id: playBottomBar

        property var slider: slider
        property var playControl: playControl
        property var expandButton: expandButton
        property var closeButton: closeButton
        property var playButton: playButton
        property var preButton: preButton
        property var lrcButton: lrcButton
        property var nextButton: nextButton

        width: miniWindow.width
        height: miniWindow.height
        color: "transparent"

        Column {

            id: playBottomBarLayout

            spacing: 2

            Rectangle {
                id: playControl

                property int iconsWidth: 30
                property int iconsHeight: iconsWidth
                property bool playing: false

                width: playBottomBar.width
                height: playBottomBar.height - slider.height - playBottomBarLayout.spacing
                color: "transparent"

                signal played(bool isPlaying)


                Column {

                    Rectangle {

                        id: topRect

                        width: playBottomBar.width
                        height: 30

                        color: "transparent"

                        Row {
                            anchors.fill: parent
                            spacing: (playControl.width - 3 * playControl.iconsWidth) / 2

                            Rectangle {
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight
                                color: "transparent"
                            }

                            DExpandNormalButton{
                                id: expandButton
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked:{
                                    miniWindow.expandNoraml();
                                }
                            }

                            DCloseLightButton{
                                id: closeButton
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked: {
                                    miniWindow.closed();
                                }
                            }
                        }
                    }

                    Rectangle{

                        id: middleRect

                        width: playBottomBar.width
                        height: playControl.height - topRect.height - bottomRect.height

                        color: "transparent"

                        Row {
                            anchors.centerIn: parent

                            DPlayButton{
                                id: playButton
                                playing: playControl.playing

                                width: playControl.iconsWidth * 2
                                height: playControl.iconsHeight * 2
                            }
                        }
                    }

                    Rectangle {

                        id: bottomRect

                        width: playBottomBar.width
                        height: 30

                        color: "transparent"

                        Row {
                            anchors.fill: parent
                            spacing: (playControl.width - 3 * playControl.iconsWidth) / 2

                            DPreButton{
                                id: preButton
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked:{
                                    miniWindow.preMusic();
                                }
                            }

                            DLrcButton{
                                id: lrcButton

                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked: {
                                    miniWindow.lrcShow();
                                }
                            }

                            DNextButton{
                                id: nextButton
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked: {
                                    miniWindow.nextMusic();
                                }
                            }
                        }
                    }
                }
            }

            SimpleSlider {
                id: slider
                objectName: 'simpleSlider'

                progressBarColor: '#0AB9E2'
                width: playBottomBar.width
                height: 6
                color: "transparent"
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