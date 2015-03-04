import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtWebEngine 1.0

Rectangle {
    width: 800
    height: 600
    visible: true
    WebEngineView {
        anchors.fill: parent
        url: "http://10.0.0.153:8096"
    }
}