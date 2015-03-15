import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
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
    property var volumeSlider: volumeSlider
    property bool playing: false
    property double volumeValue


    property var contentWidth: playBottomBar.width - 2 * playBottomBar.margins
    property var margins: 29
    property var spacing: 10

    height: musicInfo.height + slider.height + musicBar.height + spaceRect.height + 2 * playBottomBar.spacing + playBottomBar.margins


    signal preMusic()
    signal played(bool isPlaying)
    signal nextMusic()
    signal volumeChanged(double value)
    signal muted(bool muted)
    signal playbackModeChanged(int playbackMode)

    function updateCoverImage(url){
        musicInfo.musicImage.source = url;
    }

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
                color: "transparent"

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
                height: 40

                border.color: "white"
                border.width: 0

                color: playBottomBar.color
                Row{
                    anchors.fill: parent

                    Rectangle {
                        id: playControl

                        property int iconsWidth: 40 
                        property int iconsHeight: iconsWidth
                        

                        width: musicBar.width / 2
                        height: musicBar.height
                        color: playBottomBar.color

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
                        color: playBottomBar.color

                        Row {
                            id: rowLayout
                            anchors.centerIn: parent

                            DVolumeButton{
                                id: volumeButton
                                width: musicToolbar.iconsWidth
                                height: musicToolbar.iconsHeight


                                function toggleSlider(){
                                    if (volumeButton.switchflag){
                                        volumeBar.visible = true;
                                        cycleButton.opacity = 0;
                                        cycleButton.disabled = true;
                                        lrcButton.opacity = 0;
                                        lrcButton.disabled = true;
                                    }else{
                                        volumeBar.visible = false;
                                        cycleButton.opacity = 1;
                                        cycleButton.disabled = false;
                                        lrcButton.opacity = 1;
                                        lrcButton.disabled = false;
                                    }
                                }

                                function hideSlider(){
                                    volumeBar.visible = false;
                                    cycleButton.opacity = 1;
                                    cycleButton.disabled = false;
                                    lrcButton.opacity = 1;
                                    lrcButton.disabled = false;
                                }

                                onHovered:{
                                    volumeTimer.restart();
                                }

                                onExited:{
                                    volumeTimer.stop();
                                }

                                onClicked:{
                                    switchflag = !switchflag;
                                    playBottomBar.muted(!switchflag);
                                    toggleSlider();
                                }
                            }

                            DCycleButton{
                                id: cycleButton
                                width: musicToolbar.iconsWidth
                                height: musicToolbar.iconsHeight

                                onPlaybackModeChanged:{
                                    playBottomBar.playbackModeChanged(playbackMode);
                                }
                            }

                            DLrcButton{
                                id: lrcButton
                                width: musicToolbar.iconsWidth
                                height: musicToolbar.iconsHeight
                            }
                        }
                        Rectangle {
                            id: volumeBar
                            x: rowLayout.x
                            y: rowLayout.y
                            height: rowLayout.height
                            width: volumeButton.width + cycleButton.width + lrcButton.width + 3 * rowLayout.spacing
                            color: 'transparent'

                            visible: false

                            Row{
                                anchors.fill: parent
                                Rectangle{
                                    width: volumeButton.width
                                    height: rowLayout.height
                                    color: 'transparent'
                                }

                                Rectangle{
                                    id: volumeRect
                                    width:volumeBar.width - volumeButton.width
                                    height: rowLayout.height
                                    color: 'transparent'
                                    Slider {
                                        id: volumeSlider
                                        anchors.centerIn: parent
                                        width: parent.width
                                        height: 6
                                        onValueChanged:{
                                            playBottomBar.volumeChanged(value);
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                propagateComposedEvents: true
                                onExited: {
                                    volumeButton.hideSlider();
                                }

                                onPressed:{
                                    mouse.accepted = false;
                                }

                                onClicked:{
                                    mouse.accepted = false;
                                }
                                onDoubleClicked:{
                                    mouse.accepted = false;
                                }

                                onPositionChanged:{
                                    mouse.accepted = false;
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: spaceRect
                width: playBottomBar.contentWidth
                height: 60 - musicBar.height
                color: "transparent"
            }
        }
    }

    Timer {
        id: volumeTimer
        interval: 500
        running: false
        repeat: false
        onTriggered:{
            volumeButton.toggleSlider();
        }
    }
}