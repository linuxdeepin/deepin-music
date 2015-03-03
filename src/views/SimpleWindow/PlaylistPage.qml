import QtQuick 2.0
import Deepin.Widgets 1.0
import "../DMusicWidgets"

Rectangle {
    
    anchors.fill: parent

    color: "transparent"


    Component {
        id: contactDelegate
        Rectangle {
            id: mediaItem

            property var playButton: playButton

            width: parent.width
            height: 40

            color: "transparent"
            Row {

                Rectangle {
                    width: 40
                    height: 40

                    color: "transparent"

                    Rectangle {
                        id: waveBar

                        property var active: false

                        anchors.centerIn: parent
                        z: 0
                        width: 30
                        height: 40

                        visible: false

                        color: "transparent"

                        Row {
                            anchors.centerIn: parent
                            spacing: 2
                            Rectangle {
                                width: (waveBar.width - 3 * 2) / 4
                                height: waveBar.height - y
                                color: "green"
     
                                NumberAnimation on height { 
                                    loops: Animation.Infinite
                                    running: waveBar.active
                                    from :waveBar.height
                                    to: 0
                                    duration: 1000 
                                }
                            }
                            Rectangle{
                                width: (waveBar.width - 3 * 2) / 4
                                height: waveBar.height - y
                                color: "green"
                                NumberAnimation on height { 
                                    loops: Animation.Infinite
                                    running: waveBar.active
                                    from :waveBar.height
                                    to: 0
                                    duration: 1200 
                                }
                            }
                            Rectangle{
                                width: (waveBar.width - 3 * 2) / 4
                                height: waveBar.height - y
                                color: "green"
                                NumberAnimation on height { 
                                    loops: Animation.Infinite
                                    running: waveBar.active
                                    from :waveBar.height
                                    to: 0
                                    duration: 2000 
                                }
                            }
                            Rectangle{
                                width: (waveBar.width - 3 * 2) / 4
                                height: waveBar.height - y
                                color: "green"
                                NumberAnimation on height { 
                                    loops: Animation.Infinite
                                    running: waveBar.active
                                    from :waveBar.height
                                    to: 0
                                    duration: 1600 
                                }
                            }
                        }
                    }

                    DPlayButton {
                        id: playButton
                        anchors.fill: parent
                        width: 40
                        height: 40
                        visible: false
                    }


                }
                

                Text { 
                    width: mediaItem.width - playButton.width
                    height: mediaItem.height
                    verticalAlignment: Text.AlignVCenter
                    text: '<b>Name:</b> ' + index
                    color: "gray"
                }
            }


            states: State {
                name: "Current"
                when: mediaItem.ListView.isCurrentItem
                PropertyChanges { target: waveBar; active: true ; visible: true}
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    playButton.visible = true
                    // waveBar.visible = false
                }
                onExited: {
                    playButton.visible = false

                }
                onClicked: {
                    mediaItem.ListView.view.currentIndex = index;
                    playButton.visible = false
                }
            }
        }
    }

    Component {
        id: highlight
        Rectangle {
            width: 20
            color: "green"
            radius: 5
        }
    }

    ListView {
        anchors.fill: parent
        model: 100
        delegate: contactDelegate
        // highlight: highlight
        focus: true

        DScrollBar {
            flickable: parent
            inactiveColor: 'black'
        }
    }
}