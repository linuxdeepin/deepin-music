import QtQuick 2.3
import QtQml.Models 2.1
import DMusic 1.0

Rectangle{
    id: root
    anchors.fill: parent

    property var count
    property int currentIndex


    ListModel {
        id: navgationModel
        ListElement {
            name: "基础设置"
        }
        ListElement {
            name: "快捷键"
        }
        ListElement {
            name: "桌面歌词"
        }
        ListElement {
            name: "下载"
        }
        ListElement {
            name: "关于"
        }
    }

    Row {
        anchors.fill: parent
        anchors.topMargin: 20
        Rectangle {
            id: navgationBar
            width: 120
            height: root.height
            Component{
                id: delegateItem
                Rectangle {
                    id: delegateRect
                    width: parent.width
                    height: 24
                    Text {
                        id: navText
                        anchors.fill: parent
                        anchors.leftMargin: 24
                        color: "#3a3a3a"
                        horizontalAlignment: Text.AlignLeft
                        text: name
                    }
                    states: [
                        State {
                            name: "Current"
                            when: delegateRect.ListView.isCurrentItem
                            PropertyChanges { target: navText; color: "#4ba3fb"}
                        }
                    ]
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.currentIndex  = index
                            view.currentIndex = index
                        }
                    }
                }
            }

            ListView {
                id: navgationListView
                width: navgationBar.width
                height: navgationListView.count * 50
                snapMode: ListView.SnapOneItem
                highlightMoveDuration: 1
                currentIndex: root.currentIndex
                focus: true
                model: navgationModel
                delegate: delegateItem
                // highlightRangeMode: ListView.StrictlyEnforceRange

                // Component.onCompleted: positionViewAtIndex(0, ListView.Beginning)
                // Component.onCompleted: {
                //     navgationListView.currentIndex = -1;
                // }
            }
        }

        ObjectModel {
            id: itemModel

            BasicSettingSection {
                id: basicSettingSection
                width: view.width; height: view.height - 200

                subject: navgationModel.get(0).name
            }
            KeyBindingSection {
                id: keyBindingSection
                width: view.width; height: view.height - 300
                subject: navgationModel.get(1).name
            }
            DesktopLRCSection {
                id: desktopLRCSection
                width: view.width; height: view.height
                subject: navgationModel.get(2).name
            }

            DownloadSection {
                id: downloadSection
                width: view.width; height: view.height
                subject: navgationModel.get(3).name
            }

            AboutSection {
                id: aboutSection
                width: view.width; height: view.height
                subject: navgationModel.get(4).name
            }

        }

        ListView {
            id: view
            width: root.width - navgationBar.width
            height: root.height
            model: itemModel
            clip: true
            highlightMoveDuration: 1
            highlightRangeMode: ListView.StrictlyEnforceRange

            // currentIndex: root.currentIndex
            // onCurrentIndexChanged: {
            //     if (root.currentIndex != currentIndex) {
            //         root.currentIndex = currentIndex
            //     }
            // }

            // function scrollTo(sectionId) {
            //     var children = col.visibleChildren
            //     for (var i = 0; i < children.length; i++) {
            //         if (children[i].sectionId == sectionId) {
            //             contentY = children[i].y
            //         }
            //     }
            // }

            // onContentYChanged: {
            //     print(contentY, root.height)
            //     // if (!flicking) return

            //     // if(atYEnd) {
            //     //     root.currentSectionId = col.visibleChildren[col.visibleChildren.length - 1].sectionId
            //     // } else {
            //     //     var currentTopItem = col.childAt(50, contentY)
            //     //     root.currentSectionId = currentTopItem ? currentTopItem.sectionId : col.visibleChildren[0].sectionId
            //     // }
            // }


            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                hoverEnabled: false
                onWheel: {
                    wheel.accepted = false;
                    if(wheel.angleDelta.y < 0){
                        if (view.currentIndex < view.count - 1){
                            view.currentIndex = view.currentIndex + 1;
                        }
                    }
                    // else{
                    //     if (view.currentIndex > 0){
                    //         view.currentIndex = view.currentIndex - 1;
                    //     }
                    // }
                    root.currentIndex = view.currentIndex;

                    // print(view.contentY)
                }
            }



            // Component.onCompleted: positionViewAtIndex(0, ListView.Beginning)
        }
    }

    Component.onCompleted: {
        root.currentIndex = 0;
    }
}
