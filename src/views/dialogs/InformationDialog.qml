import QtQuick 2.3
import QtGraphicalEffects 1.0
import DMusic 1.0
import "../DMusicWidgets"
import Deepin.Widgets 1.0

DGlowRectangle {

    width: 304
    height: 604

    contentItem: Rectangle {
        id: contentItem
        anchors.fill: parent
        anchors.margins: 2
        radius: 4
        color: "white"

        Column {
            anchors.fill: parent
            
            Rectangle {
                id: titleBar
                width: parent.width
                height: closeButton.height
                color: "transparent"
                DCloseButton{
                    id: closeButton
                    width: 25
                    height: 25

                    anchors.right: parent.right
                    anchors.top: parent.top
                    onClicked:{
                        SignalManager.dialogClosed();
                    }
                }
            }

            Rectangle {
                id: box
                width: parent.width
                height: 575
                Column {
                    anchors.fill: parent
                    Rectangle {
                        width: parent.width
                        height:150
                        Column{
                            anchors.centerIn: parent
                            spacing: 20

                            Image {
                                asynchronous: true
                                width: 108
                                height: 108
                                source: QmlDialog.songObj.cover
                            }

                            DText {
                                width: parent.width
                                height: 12
                                color: "#888888"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                                text: QmlDialog.songObj.title
                            }
                        }
                    }

                    BaseSection {
                        id: section1
                        width: parent.width
                        expandHeight: 250
                        state: 'expanded'
                        sectionTitle: '1'
                        contentItem: Rectangle {
                            anchors.fill: parent
                            color: "red"
                        }
                        onStateChanged:{
                            if (state == 'expanded'){
                                section2.state = 'closed';
                                section3.state = 'closed';
                                section4.state = 'closed';
                            }
                        }
                    }

                    BaseSection {
                        id: section2
                        width: parent.width
                        expandHeight: 200
                        state: 'closed'
                        sectionTitle: '2'
                        contentItem: Rectangle {
                            anchors.fill: parent
                            color: "red"
                        }
                        onStateChanged:{
                            if (state == 'expanded'){
                                section1.state = 'closed';
                                section3.state = 'closed';
                                section4.state = 'closed';
                            }
                        }
                    }

                    BaseSection {
                        id: section3
                        width: parent.width
                        expandHeight: 150
                        state: 'closed'
                        sectionTitle: '3'
                        contentItem: Rectangle {
                            anchors.fill: parent
                            color: "red"
                        }
                        onStateChanged:{
                            if (state == 'expanded'){
                                section1.state = 'closed';
                                section2.state = 'closed';
                                section4.state = 'closed';
                            }
                        }
                    }

                    BaseSection {
                        id: section4
                        width: parent.width
                        expandHeight: 200
                        state: 'closed'
                        sectionTitle: '4'
                        contentItem: Rectangle {
                            anchors.fill: parent
                            color: "red"
                        }
                        onStateChanged:{
                            if (state == 'expanded'){
                                section1.state = 'closed';
                                section2.state = 'closed';
                                section3.state = 'closed';
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: parent.height - section1.height - section2.height - section3.height - section4.height
                        color: "white"
                    }
                }
            }
        }
    }
}
