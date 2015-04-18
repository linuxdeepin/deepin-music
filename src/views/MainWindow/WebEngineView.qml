import QtQuick 2.3

Rectangle {
    property var url

    signal linkHovered(string link)
    signal javaScriptConsoleMessage(int level, string message)
    signal navigationRequested(var reuqest)
    signal loadingChanged(var loadRequest)
}