import QtQuick 2.3
import QtGraphicalEffects 1.0
import DMusic 1.0
import "../DMusicWidgets"
import Deepin.Widgets 1.0

DWindowFrame {

    width: 304
    height: 636

    content: Rectangle {
        id: contentItem
        anchors.fill: parent
        anchors.margins: 2
        radius: 4

        Column {
            anchors.fill: parent
            spacing: 0
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
                height: 606 - 25

                Column {
                    anchors.fill: parent
                    Rectangle {
                        width: parent.width
                        height:170
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
                        expandHeight: 240
                        state: 'expanded'
                        sectionTitle: I18nWorker.info_songAbstract
                        contentItem: SectionAbstractPage {
                            anchors.fill: parent
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
                        sectionTitle: I18nWorker.info_detail
                        contentItem: SectionDetailPage {
                            anchors.fill: parent
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
                        expandHeight: 205
                        state: 'closed'
                        sectionTitle: I18nWorker.info_lyric
                        contentItem: SectionLyricPage {
                            anchors.fill: parent
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
                        expandHeight: 100
                        state: 'closed'
                        sectionTitle: I18nWorker.info_options
                        contentItem: SectionOptionsPage {
                            anchors.fill: parent
                        }
                        onStateChanged:{
                            if (state == 'expanded'){
                                section1.state = 'closed';
                                section2.state = 'closed';
                                section3.state = 'closed';
                            }
                        }
                    }

                    // Rectangle {
                    //     width: parent.width
                    //     height: parent.height - section1.height - section2.height - section3.height - section4.height - 20
                    //     color: "white"
                    // }
                }
            }
        }
    }
}
