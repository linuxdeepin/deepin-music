import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import DMusic 1.0
import "../DMusicWidgets"

Item {
    id: playBottomBar
    property var viewID: 'PlayBottomBar'

    property var slider: slider
    property var musicImage: musicImage
    property var musicInfo: musicInfo
    property var playControl: playControl
    property var musicToolbar: musicToolbar
    property var preButton: preButton
    property var playButton: playButton
    property var nextButton: nextButton
    property var volumeButton: volumeButton
    property var cycleButton: cycleButton
    property var lrcButton: lrcButton
    property var playlistButton: playlistButton
    property var volumeSlider: volumeSlider
    property bool playing: false
    property double volumeValue

    property real playerDuration: 0
    property real playerPosition: 0

    property color color: "white"
    property color styleColor: "#0AB9E2"

    property color boderColor: 'white'
    // property color boderColor: "transparent"
    property int boderWidth: 0

    property int musicInfoWidth: 350

    property color spacingColor: "transparent"
    // property color spacingColor: "transparent"


    signal preMusic()
    signal played(bool isPlaying)
    signal nextMusic()
    signal volumeChanged(double value)
    signal muted(bool muted)
    signal playbackModeChanged(int playbackMode)


    function updateCoverImage(url){
        musicImage.source = url;
    }
 
    function updateMusicName(name){
        musicNameText.text = name;
    }

    function updateArtistName(name){
        artistNameText.text = name;
    }

    function updatePlayTime(timeString){
        timeText.text = timeString;
    }


    Column {

        anchors.fill: playBottomBar
        anchors.margins: 0

        spacing: 0

        SimpleSlider {
            id: slider
            objectName: 'simpleSlider'

            progressBarColor: '#0AB9E2'
            width: playBottomBar.width
            height: 6
        }
        Row {
            id: bottomBar
            
            height: playBottomBar.height - slider.height
            
            spacing: 0

            Rectangle {
                id: musicInfo
                width: playBottomBar.musicInfoWidth
                height: bottomBar.height
                border.color: playBottomBar.boderColor
                border.width: playBottomBar.boderWidth
                color: playBottomBar.color

                Row {

                    anchors.centerIn: musicInfo

                    Rectangle{
                        id: leftSpace
                        width: 25
                        height: parent.height
                        color: playBottomBar.spacingColor
                    }

                    MusicImage {
                        id: musicImage
                        source: "../../skin/images/bg2.jpg"
                    }

                    Rectangle{
                        id : rightSpace
                        width: 15
                        height: parent.height
                        color: playBottomBar.spacingColor
                    }

                    Rectangle {

                        id: musicText
                        width: musicInfo.width - musicImage.width - leftSpace.width - rightSpace.width -  2 * playBottomBar.boderWidth
                        height: 60
                        color: playBottomBar.color

                        Column {

                            anchors.centerIn: parent
                            anchors.margins: 20

                            Text {
                                id: musicNameText
                                width: musicText.width
                                height: 20
                                color: "white"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: 'LIFE'
                            }

                            Text {
                                id:artistNameText
                                width: musicText.width
                                height: 20
                                color: "white"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: 'Kenichior Nishihara'
                            }

                            Row {

                                Text {
                                    id: timeText
                                    width: musicText.width - 160
                                    height: 20
                                    color: "white"
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    text: '02:06/04:15'
                                }

                                DStarButton{
                                    id: musicStar
                                    width: 20
                                    height: 20
                                }

                                DDownloadButton{
                                    id: musicDownload
                                    width: 20
                                    height: 20
                                }

                                Rectangle{
                                    width: parent.width - timeText.width - musicStar.width - musicDownload.width 
                                    height: parent.height
                                }
                            }
                        }
                    }
                }

            }

            Rectangle {
                id: playControl

                property int iconsWidth: 60 
                property int iconsHeight: iconsWidth
                

                width: playBottomBar.width - musicInfo.width - musicToolbar.width
                height: bottomBar.height
                color: playBottomBar.color

                border.color: playBottomBar.boderColor
                border.width: playBottomBar.boderWidth


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

            Rectangle{
                id: musicToolbar
                width: musicInfo.width

                property int iconsWidth: 50
                property int iconsHeight: iconsWidth

                height: bottomBar.height
                color: playBottomBar.color

                border.color: playBottomBar.boderColor
                border.width: playBottomBar.boderWidth

                Row {
                    id: rowLayout
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 20

                    spacing: 5

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

                    DCycleButton {
                        id: cycleButton
                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight

                        onPlaybackModeChanged:{
                            playBottomBar.playbackModeChanged(playbackMode);
                        }
                    }

                    DLrcButton {
                        id: lrcButton
                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
                    }

                    DPlaylistButton {
                        id: playlistButton
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
                                    playBottomBar.volumeChanged(value)
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

    Timer {
        id: volumeTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered:{
            volumeButton.toggleSlider();
        }
    }
}
