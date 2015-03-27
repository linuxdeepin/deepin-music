import QtQuick 2.4

ListModel {
    ListElement {
        name: {return I18nWorker.artist}
    }
    ListElement {
        name: "专辑"
    }
    ListElement {
        name: "歌曲"
    }
    ListElement {
        name: '文件夹'
    }

    Component.onCompleted: {
        print(I18nWorker.artist)
    }
}