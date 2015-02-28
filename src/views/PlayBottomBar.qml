import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import DMusic 1.0

Item {
    id: playBottomBar
    property var viewID: 'PlayBottomBar'

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
    property var playlistButton: playlistButton

    property real playerDuration: 0
    property real playerPosition: 0

    property color color: "white"
    property color styleColor: "#0AB9E2"

    property color boderColor: 'white'
    // property color boderColor: "transparent"
    property int boderWidth: 2

    property int musicInfoWidth: 350

    property color spacingColor: "#2A3B4D"
    // property color spacingColor: "transparent"

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
                        width: 30
                        height: parent.height
                        color: playBottomBar.spacingColor
                    }

                    Rectangle{
                        id: musicImage
                        width: 60
                        height: 60


                        BorderImage {
                            anchors.fill: musicImage
                            horizontalTileMode: BorderImage.Stretch
                            verticalTileMode: BorderImage.Stretch
                            source: "../skin/images/bg2.jpg"
                        }
                    }

                    Rectangle{
                        width: 10
                        height: parent.height
                        color: playBottomBar.spacingColor
                    }

                    Rectangle {

                        id: msuicText
                        width: musicInfo.width - musicImage.width - 40 -  2 * playBottomBar.boderWidth
                        height: 60
                        color: playBottomBar.color

                        Column {

                            anchors.centerIn: parent
                            anchors.margins: 20

                            Text {
                                width: msuicText.width
                                height: 20
                                color: "white"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: 'LIFE'
                            }

                            Text {
                                width: msuicText.width
                                height: 20
                                color: "white"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: 'Kenichior Nishihara'
                            }

                            Row {

                                Text {
                                    id: timeText
                                    width: msuicText.width - 160
                                    height: 20
                                    color: "white"
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    text: '02:06/04:15'
                                }

                                DStarButton{
                                    id: musicstar
                                    width: 20
                                    height: 20
                                }

                                DDownloadButton{
                                    id: musicdownload
                                    width: 20
                                    height: 20
                                }

                                Rectangle{
                                    width: parent.width - timeText.width - musicstar.width - musicdownload.width 
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
                property bool playing: false

                width: playBottomBar.width - musicInfo.width - musicToolbar.width
                height: bottomBar.height
                color: playBottomBar.color

                border.color: playBottomBar.boderColor
                border.width: playBottomBar.boderWidth

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
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 20

                    spacing: 5

                    DVolumeButton {
                        id: volumeButton
                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
                    }

                    DCycleButton {
                        id: cycleButton
                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
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
            }
        }
    }
}
