import QtQuick 2.4

Rectangle {
    id: catgoryCombox

    property var view: catgoryView
    property int currentIndex: -1
    property string currentText

    width: 100
    height: 100
    radius: 4
    border.width: 1
    border.color: "gray"

    signal itemClicked()

    Component {
        id: catgoryDelegate
        Text {
            id: item
            width: item.ListView.width
            height: 20
            text: name
            color: "#252525"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered:{
                    item.color = "#2ca7f8"
                }
                onExited:{
                    item.color = "#252525"
                }
                onClicked: {
                    catgoryCombox.currentIndex = index;
                    catgoryCombox.currentText = name;
                    catgoryCombox.itemClicked();
                }
            }
        }
    }

    ListView {
        id: catgoryView
        anchors.margins: 10
        anchors.fill: parent
        spacing: 2
        interactive: false
        model: MusicCatgoryModel{}
        delegate: catgoryDelegate
        currentIndex: 0
    }
    visible: false

    Component.onCompleted: {
        catgoryCombox.currentText = catgoryView.model.get(0).name;
        catgoryCombox.currentIndex = 0
    }
}