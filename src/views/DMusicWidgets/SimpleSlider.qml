import QtQuick 2.3

Rectangle {
    id: slider

    property color progressBarColor: '#0AB9E2'
    property color progressBufferBarColor: 'gray'
    property var value

    width: parent.width
    height: 6

    color: "lightgray"

    function updateSlider(rate) {
        progressBar.width = slider.width * rate
    }

    function updateBufferSlider(rate) {
        progressBufferBar.width = slider.width * rate
    }

    signal sliderRateChanged(double rate)

    Rectangle {
        id: progressBufferBar
        anchors.left: slider.left
        anchors.top: slider.top
        anchors.bottom: slider.bottom
        width: 0
        color: slider.progressBufferBarColor
    }

    Rectangle {
        id: progressBar
        anchors.left: slider.left
        anchors.top: slider.top
        anchors.bottom: slider.bottom
        width: 0
        color: slider.progressBarColor
    }

    MouseArea {
        anchors.fill: slider
        onClicked: {
            slider.sliderRateChanged(mouse.x / slider.width);
            slider.value = mouse.x / slider.width;
        }
    }

}