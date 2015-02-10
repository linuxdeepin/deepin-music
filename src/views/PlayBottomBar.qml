import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import DMusic 1.0

Item {
    id: playBottomBar
    property var viewID: 'PlayBottomBar'

    property var slider: slider
    property var musicinfo: musicinfo
    property var playControl: playControl
    property var musicToolbar: musicToolbar
    property var playButton: playButton

    property real playerDuration: 0
    property real playerPosition: 0

    property color color: "white"
    property color styleColor: "#0AB9E2"

    // property color boderColor: 'white'
    property color boderColor: "transparent"
    property int boderWidth: 2

    property int musicinfoWidth: 350

    // property color spacingColor: "#2A3B4D"
    property color spacingColor: "transparent"


    Column {

        anchors.fill: playBottomBar
        anchors.margins: 0

        spacing: 0

        Rectangle {
            id: slider
            width: playBottomBar.width
            height: 6
            
            color: "Gray"
            
            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                
                width: playBottomBar.playerDuration > 0 ? parent.width * playBottomBar.playerPosition/ playBottomBar.playerDuration: 0
                
                color: playBottomBar.styleColor
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (player.seekable)
                        player.seek(playBottomBar.playerDuration * mouse.x / slider.width)
                }
            }
        }
        Row {
            id: bottomBar
            
            height: playBottomBar.height - slider.height
            
            spacing: 0

            Rectangle {
                id: musicinfo
                width: playBottomBar.musicinfoWidth
                height: bottomBar.height
                border.color: playBottomBar.boderColor
                border.width: playBottomBar.boderWidth
                color: playBottomBar.color

                Row {

                    anchors.centerIn: musicinfo

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
                        width: musicinfo.width - musicImage.width - 40 -  2 * playBottomBar.boderWidth
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

                                DIconButton{
                                    id: musicstar
                                    width: 20
                                    height: 20
                                    normal_image: '../skin/icons/dark/appbar.heart.outline.png'
                                    hover_image: '../skin/icons/dark/appbar.heart.outline.png'
                                    pressed_image: '../skin/icons/light/appbar.heart.outline.png'
                                    disabled_image: '../skin/icons/dark/appbar.heart.outline.png'
                                }

                                DIconButton{
                                    id: musicdownload
                                    width: 20
                                    height: 20
                                    normal_image: '../skin/icons/dark/appbar.arrow.down.png'
                                    hover_image: '../skin/icons/dark/appbar.arrow.down.png'
                                    pressed_image: '../skin/icons/light/appbar.arrow.down.png'
                                    disabled_image: '../skin/icons/dark/appbar.arrow.down.png'
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

                width: playBottomBar.width - musicinfo.width - musicToolbar.width
                height: bottomBar.height
                color: playBottomBar.color

                border.color: playBottomBar.boderColor
                border.width: playBottomBar.boderWidth

                Row {
                    anchors.centerIn: parent

                    DIconButton{
                        
                        width: playControl.iconsWidth
                        height: playControl.iconsHeight
                        normal_image: '../skin/icons/dark/appbar.navigate.previous.png'
                        hover_image: '../skin/icons/dark/appbar.navigate.previous.png'
                        pressed_image: '../skin/icons/light/appbar.navigate.previous.png'
                        disabled_image: '../skin/icons/dark/appbar.navigate.previous.png'
                    }

                    DIconButton{
                        id: playButton
                        property bool playing: false

                        signal played(bool flag)

                        width: playControl.iconsWidth
                        height: playControl.iconsHeight
                        normal_image: playButton.playing ?  '../skin/icons/dark/appbar.control.pause.png' : '../skin/icons/dark/appbar.control.play.png'
                        hover_image: playButton.playing ? '../skin/icons/dark/appbar.control.pause.png': '../skin/icons/dark/appbar.control.play.png'
                        pressed_image: playButton.playing ? '../skin/icons/dark/appbar.control.pause.png' : '../skin/icons/light/appbar.control.play.png'
                        disabled_image: playButton.playing ? '../skin/icons/dark/appbar.control.pause.png' : '../skin/icons/dark/appbar.control.play.png'

                        onClicked:{
                            playButton.playing = !playButton.playing;
                            playButton.played(playButton.playing);
                        }
                    }

                    DIconButton{

                        width: playControl.iconsWidth
                        height: playControl.iconsHeight
                        normal_image: '../skin/icons/dark/appbar.navigate.next.png'
                        hover_image: '../skin/icons/dark/appbar.navigate.next.png'
                        pressed_image: '../skin/icons/light/appbar.navigate.next.png'
                        disabled_image: '../skin/icons/dark/appbar.navigate.next.png'
                    }

                }

                
            }

            Rectangle{
                id: musicToolbar
                width: musicinfo.width

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

                    DIconButton{
                        
                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
                        normal_image: '../skin/icons/dark/appbar.sound.3.png'
                        hover_image: '../skin/icons/dark/appbar.sound.3.png'
                        pressed_image: '../skin/icons/light/appbar.sound.3.png'
                        disabled_image: '../skin/icons/dark/appbar.sound.3.png'
                    }

                    DIconButton{

                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
                        normal_image: '../skin/icons/dark/appbar.refresh.png'
                        hover_image: '../skin/icons/dark/appbar.refresh.png'
                        pressed_image: '../skin/icons/light/appbar.refresh.png'
                        disabled_image: '../skin/icons/dark/appbar.refresh.png'
                    }

                    DIconButton{

                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
                        normal_image: '../skin/icons/dark/appbar.adobe.lightroom.png'
                        hover_image: '../skin/icons/dark/appbar.adobe.lightroom.png'
                        pressed_image: '../skin/icons/light/appbar.adobe.lightroom.png'
                        disabled_image: '../skin/icons/dark/appbar.adobe.lightroom.png'
                    }

                    DIconButton{

                        width: musicToolbar.iconsWidth
                        height: musicToolbar.iconsHeight
                        normal_image: '../skin/icons/dark/appbar.list.png'
                        hover_image: '../skin/icons/dark/appbar.list.png'
                        pressed_image: '../skin/icons/light/appbar.list.png'
                        disabled_image: '../skin/icons/dark/appbar.list.png'
                    }

                }
            }
        }
    }
}
