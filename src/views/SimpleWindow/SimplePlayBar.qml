import QtQuick 2.3
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.1
import DMusic 1.0
import "../DMusicWidgets"
import "../DMusicWidgets/SimplePlayer"
import "../DMusicWidgets/CustomSlider"

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

    property var title
    property var artist
    property var cover

    property int iconsWidth: 40 


    property var contentWidth: playBottomBar.width - 2 * playBottomBar.margins
    property var margins: 29
    property var spacing: 10

    height: musicInfo.height + slider.height + musicBar.height + spaceRect.height + 2 * playBottomBar.spacing + playBottomBar.margins

    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0) }
        GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.6) }
    }

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

                title: playBottomBar.title
                artist: playBottomBar.artist
                cover: playBottomBar.cover

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

                color: 'transparent'
                Row{
                    id: musicBarLayout
                    anchors.fill: parent
                    spacing: 38
                    Rectangle {
                        id: playControl

                        property int iconsWidth: playBottomBar.iconsWidth 
                        property int iconsHeight: iconsWidth
                        

                        width: (musicBar.width - musicBarLayout.spacing) / 2
                        height: musicBar.height
                        color: 'transparent'

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

                        property int iconsWidth: playBottomBar.iconsWidth 
                        property int iconsHeight: iconsWidth
                        property bool playing: false

                        width: (musicBar.width - musicBarLayout.spacing) / 2
                        height: musicBar.height
                        color: 'transparent'

                        Row {
                            id: rowLayout
                            anchors.centerIn: parent

                            spacing: 0

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

                            DLoopButton{
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
                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.leftMargin: 4
                                        anchors.rightMargin: 4
                                        color: 'transparent'
                                        DSlider {
                                            id: volumeSlider
                                            anchors.centerIn: parent
                                            width: parent.width
                                            height: 6
                                            onValueChanged:{
                                                playBottomBar.volumeChanged(value)
                                            }
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