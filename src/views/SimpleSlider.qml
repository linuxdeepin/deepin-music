import QtQuick 2.0

Rectangle {
    id: slider

    property color progressBarColor: '#0AB9E2'

    width: parent.width
    height: 6

    color: "Gray"

    function updateSlider(rate) {
        progressBar.width = slider.width * rate
    }

    signal sliderRateChanged(double rate)
    
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
        }
    }
}