import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0
import Deepin.Widgets 1.0
import "../DMusicWidgets"
import "../DMusicWidgets/MiniWindow"
import "../DMusicWidgets/CustomSlider"

DMovableWindow {
    id: miniWindow

    property var constants
    property var miniWindowController: miniWindowController
    property var bgImage: bgImage
    property var slider: slider
    property bool playing: false

    color: "#282F3F"

    x: Screen.desktopAvailableWidth - 200
    y: 100

    signal closed()
    signal expandNoraml()
    signal played(bool isPlaying)
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

                width: playBottomBar.width
                height: playBottomBar.height - slider.height - playBottomBarLayout.spacing
                color: "transparent"

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

                            DCloseButton{
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
                                playing: miniWindow.playing

                                width: playControl.iconsWidth * 2
                                height: playControl.iconsHeight * 2

                                onClicked:{
                                    miniWindow.playing = !miniWindow.playing;
                                    miniWindow.played(miniWindow.playing);
                                }
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
        constants: miniWindow.constants
        bgImage: bgImage
        playBottomBar: playBottomBar
    }

    Item {
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            var shortcut = KeysUtils.keyEventToString(event.modifiers, event.key);
            if (shortcut == ConfigWorker.shortcut_miniFullMode) {
                SignalManager.miniFullToggle();
            }
        }
    }
}