import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Slider {
    style: SliderStyle {
        groove: Rectangle {
            implicitHeight: 4
            color: "white"

            Rectangle {
                width: control.value * parent.width
                height: 4
                color: "#40bafe"
            }
        }
        handle: Rectangle {
            anchors.centerIn: parent
            color: control.pressed ? "white" : "lightgray"
            // border.color: "gray"
            // border.width: 2
            implicitWidth: 12
            implicitHeight: 12
            radius: 6
        }
    }
}
