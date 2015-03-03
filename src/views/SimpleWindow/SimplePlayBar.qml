import QtQuick 2.0
import DMusic 1.0
import "../DMusicWidgets"

Rectangle{
    id: playBottomBar

    property var slider: slider
    property var musicInfo: musicInfo
    property var playControl: playControl
    property var musicToolbar: musicToolbar
    property var preButton: preButton
    property var playButton: playButton
    property var nextButton: nextButton
    property var volumeButton: volumeButton
    property var cycleButton: cycleButton
    property var lrcButton: lrcButton
    property bool playing: false


    property var contentWidth: playBottomBar.width - 2 * playBottomBar.margins
    property var margins: 25
    property var spacing: 10

    height: musicInfo.height + slider.height + musicBar.height + 2 * playBottomBar.spacing + playBottomBar.margins


    signal preMusic()
    signal played(bool isPlaying)
    signal nextMusic()

    function updateMusicName(name){
        musicInfo.musicNameText.text = name;
    }

    function updateArtistName(name){
        musicInfo.artistNameText.text = name;
    }

    function updatePlayTime(timeString){
        musicInfo.timeText.text = timeString;
    }


    Rectangle{
        anchors.fill: parent
        anchors.margins: playBottomBar.margins
        color: "transparent"
        Column{

            spacing: playBottomBar.spacing

            SimpleMusicInfo {
                id: musicInfo
                width: playBottomBar.contentWidth
                height: 60
                color: "#282F3F"

                border.color: "white"
                border.width: 0
            }

            SimpleSlider {
                id: slider
                objectName: 'simpleSlider'

                progressBarColor: '#0AB9E2'
                width: playBottomBar.contentWidth
                height: 6
            }

            Rectangle {
                id: musicBar
                width: playBottomBar.contentWidth
                height: 60

                border.color: "white"
                border.width: 0

                color: "transparent"
                Row{
                    anchors.fill: parent

                    Rectangle {
                        id: playControl

                        property int iconsWidth: 40 
                        property int iconsHeight: iconsWidth
                        

                        width: musicBar.width / 2
                        height: musicBar.height
                        color: "transparent"

                        Row {
                            anchors.centerIn: parent

                            DPreButton{
                                id: preButton
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked : {
                                    playBottomBar.preMusic();
                                }
                            }

                            DPlayButton{
                                id: playButton
                                playing: playBottomBar.playing

                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked: {
                                    playBottomBar.playing = !playBottomBar.playing;
                                    playBottomBar.played(playBottomBar.playing);
                                }
                            }

                            DNextButton{
                                id: nextButton
                                width: playControl.iconsWidth
                                height: playControl.iconsHeight

                                onClicked: {
                                    playBottomBar.nextMusic();
                                }
                            }
                        }
                    }
                    Rectangle {
                        id: musicToolbar

                        property int iconsWidth: 40 
                        property int iconsHeight: iconsWidth
                        property bool playing: false

                        width: musicBar.width / 2
                        height: musicBar.height
                        color: "transparent"

                        Row {
                            anchors.centerIn: parent

                            DVolumeButton{
                            	id: volumeButton
                                width: musicToolbar.iconsWidth
                                height: musicToolbar.iconsHeight
                            }

                            DCycleButton{
                            	id: cycleButton
                                width: musicToolbar.iconsWidth
                                height: musicToolbar.iconsHeight
                            }

                            DLrcButton{
                            	id: lrcButton
                                width: musicToolbar.iconsWidth
                                height: musicToolbar.iconsHeight
                            }
                        }
                    }
                }
            }
        }
    }
}