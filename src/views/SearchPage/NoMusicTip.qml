import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: noMusicTip
    property int topMargin: 160
    anchors.fill: parent
    visible: false

    function getTextByLength(s, l){
        if (s.length > l){
            return s.slice(0, l) + '...'
        }else{
            return s
        }
    }

    Rectangle {
       anchors.topMargin: topMargin
       anchors.fill: parent
       Column {
            spacing: 5
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: linkTipText
                color: "#535353"
                linkColor: "#31a4fa"
                font.pixelSize: 14
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: linkTipText1
                color: "#535353"
                linkColor: "#31a4fa"
                font.pixelSize: 12
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                text: "1. 检查输入的关键字是否正确"
            }

            Text {
                id: linkTipText2
                color: "#535353"
                linkColor: "#31a4fa"
                font.pixelSize: 12
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                text: "2. 缩短关键字长度"
            }
        }
    }

    Binding {
        target: linkTipText
        property: 'text'
        value: {
            var startText = "没有找到与\" <a href=\"Online\" style=\"text-decoration:none;\">";
            var endText = "</a>\" 相关的音乐";
            var result = getTextByLength(SearchWorker.keyword, 20);
            return startText + result + endText;
        }
    }
}