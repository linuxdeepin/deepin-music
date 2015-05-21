import QtQuick 2.3

Item {
    property var flickable

    function init(){
        SignalManager.lrcSetting.connect(positionLrcSetting);
    }

    function positionLrcSetting(){
        var index = 2;
        flickable.currentIndex = index;
        flickable.scrollbyIndex(index);
    }

    Component.onCompleted: {
        init();
    }
}
