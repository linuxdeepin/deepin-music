import QtQuick 2.3
import QtQuick.Controls 1.0

Item{
    id: root
    property bool disabled: false
    property bool hoverEnabled: true
    property url normal_image
    property url hover_image
    property url pressed_image
    property url disabled_image

    property string tooltip

    signal hovered
    signal clicked
    signal pressed
    signal released
    signal exited

    state: 'normal'

    MouseArea {

        id: mouseArea

        anchors.fill: parent
        enabled: !disabled
        hoverEnabled: parent.hoverEnabled
        onEntered: {
            parent.hovered();
            parent.state = "hovered"

      //    var obj = mapToItem(null, mouseX, mouseY);
            // print(obj.x, obj.y, obj.width, obj.height)
        }
        onExited: {
            parent.exited()
            parent.state = "normal"

            // showTimer.start()

        }

        onPressed:{
            parent.pressed()
            parent.state = "pressed"
        }

        onReleased:{
            parent.released()
            parent.state = "normal"
        }

        onClicked: {
            parent.clicked()
        }
    }

    Image {
        id: image
        asynchronous: true
        sourceSize.width: parent.width
        sourceSize.height: parent.height
    }

    Component {
        id: tooltip
        DToolTip {
            tooltip: root.tooltip
        }
    }

    Loader {
        id: tooltipLoader
        // active: false
    }


    Timer {
        id: showTimer
        interval: 1000
        running: false
        onTriggered:{
            if (mouseArea.containsMouse){
                tooltipLoader.sourceComponent = tooltip;
                if (tooltipLoader.item ){
                    tooltipLoader.item.visible = true;
                    tooltipLoader.item.x = Qt.globalPos.x + 10;
                    tooltipLoader.item.y = Qt.globalPos.y + 10;
                    hideTimer.restart();
                }
            }
        }
    }

    Timer {
        id: hideTimer
        interval: 2000
        running: false

        onTriggered:{
            tooltipLoader.sourceComponent = null;
            hideTimer.stop();
        }
    }

    states:[
        State{
            name: "normal"
            PropertyChanges {target: image; source: normal_image}
        },
        State{
            name: "hovered"
            PropertyChanges {target: image; source: hover_image}
        },
        State{
            name: "pressed"
            PropertyChanges {target: image; source: pressed_image}
        },
        State{
            name: "disabled"
            PropertyChanges {target: image; source: disabled_image}
        }
    ]
}
