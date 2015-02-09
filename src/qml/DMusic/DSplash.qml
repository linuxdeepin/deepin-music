import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    id: root

    property int showDuration: 500
    property int pauseDuration: 2000
    property int hideDuration: 500

    opacity: 0
    anchors.fill: parent
    color: "#345678"

    signal finished()

    SequentialAnimation {
        id: loadAnimation
        NumberAnimation { 
            target: root
            property: "opacity"; 
            to: 1; 
            duration: hideDuration 
            easing.type: Easing.InOutQuad 
        }
        PauseAnimation { duration: pauseDuration}
        NumberAnimation { 
            target: root
            property: "opacity"
            to: 0
            duration: hideDuration
            easing.type: Easing.InOutQuad 
        }
        ScriptAction {
            script: {
                root.finished()
                root.destroy()
            }
        }
    }

    Component.onCompleted: {
        loadAnimation.start()
    }
}
