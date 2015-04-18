import QtQuick 2.3

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
            text: item.ListView.view.model[index].name
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
                    catgoryCombox.currentText = item.ListView.view.model[index].name;
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
        model: MusicManageWorker.categories
        delegate: catgoryDelegate
        currentIndex: 0

        onCurrentIndexChanged:{
            catgoryCombox.currentText = catgoryView.model[currentIndex].name
        }
    }
    visible: false

    Component.onCompleted: {
        catgoryCombox.currentText = catgoryView.model[0].name;
        catgoryCombox.currentIndex = 0
    }
}