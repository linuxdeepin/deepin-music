// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQml.Models 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

DialogWindow {
    property ListModel comBoxModel: ListModel {
        ListElement { text: qsTr("Custom") }
        ListElement { text: qsTr("Monophony") }
        ListElement { text: qsTr("Classical") }
        ListElement { text: qsTr("Club") }
        ListElement { text: qsTr("Dance") }
        ListElement { text: qsTr("Full Bass") }
        ListElement { text: qsTr("Full Bass and Treble") }
        ListElement { text: qsTr("Full Treble") }
        ListElement { text: qsTr("Headphones") }
        ListElement { text: qsTr("Hall") }
        ListElement { text: qsTr("Live") }
        ListElement { text: qsTr("Party") }
        ListElement { text: qsTr("Pop") }
        ListElement { text: qsTr("Reggae") }
        ListElement { text: qsTr("Rock") }
        ListElement { text: qsTr("Ska") }
        ListElement { text: qsTr("Soft") }
        ListElement { text: qsTr("Soft Rock") }
        ListElement { text: qsTr("Techno") }
    }
    property ListModel sliderModel: ListModel {
        ListElement {name: "60"; value: 0}
        ListElement {name: "170"; value: 0}
        ListElement {name: "310"; value: 0}
        ListElement {name: "600"; value: 0}
        ListElement {name: "1k"; value: 0}
        ListElement {name: "3k"; value: 0}
        ListElement {name: "6k"; value: 0}
        ListElement {name: "12k"; value: 0}
        ListElement {name: "14k"; value: 0}
        ListElement {name: "16k"; value: 0}
    }

    property var baudsList: [[12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        [12, 0, 0, 0, 0, 0, 0, -7, -7, -7, -9],
        [6, 0, 0, 8, 5, 5, 5, 3, 0, 0, 0],
        [5, 9, 7, 2, 0, 0, -5, -7, -7, 0, 0],
        [5, -8, 9, 9, 5, 1, -4, -8, -10, -11, -11],
        [4, 7, 5, 0, -7, -4, 1, 8, 11, 12, 12],
        [3, -9, -9, -9, -4, 2, 11, 16, 16, 16, 16],
        [4, 4, 11, 5, -3, -2, 1, 4, 9, 12, 14],
        [5, 10, 10, 5, 5, 0, -4, -4, -4, 0, 0],
        [7, -4, 0, 4, 5, 5, 5, 4, 2, 2, 2],
        [6, 7, 7, 0, 0, 0, 0, 0, 0, 7, 7],
        [6, -1, 4, 7, 8, 5, 0, -2, -2, -1, -1],
        [8, 0, 0, 0, -5, 0, 6, 6, 0, 0, 0],
        [5, 8, 4, -5, -8, -3, 4, 8, 11, 11, 11],
        [6, -2, -4, -4, 0, 4, 5, 8, 9, 11, 9],
        [5, 4, 1, 0, -2, 0, 4, 8, 9, 11, 12],
        [7, 4, 4, 2, 0, -4, -5, -3, 0, 2, 8],
        [5, 8, 5, 0, -5, -4, 0, 8, 9, 9, 8],
    ]
    property var curList: [12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    property var customList: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    property bool switched: false
    property int  curEQ: 0

    maximumWidth: 572
    maximumHeight: 426
    minimumWidth: 572
    minimumHeight: 426
    header: DialogTitleBar {
        enableInWindowBlendBlur: false
    }

    Column {
        width: parent.width - 20
        height: parent.height - 20
        spacing: 40
        leftPadding: 10

        Rectangle {
            id: header
            width: parent.width
            height: 46
            color: "transparent"

            Row {
                width: parent.width
                height: parent.height
                spacing: 20
                Rectangle {
                    id: switchArea
                    width: 108
                    height: parent.height
                    color: "transparent"
                    Row {
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            width: contentWidth < 55 ? 55 : contentWidth
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Equalizer")
                        }
                        Switch {
                            id: switchBtn
                            anchors.verticalCenter: parent.verticalCenter
                            checked: switched
                            onCheckedChanged: {
                                if (checked === true) {
                                    selectComBox.enabled = true
                                    if (selectComBox.currentIndex === 0) {
                                        saveBtn.enabled = true
                                    }
                                    resetBtn.enabled = true
                                    content.enabled = true
                                    Presenter.setEQEnable(true)
                                    //Presenter.setEQCurMode(curEQ)
                                    Presenter.setValueToSettings("equalizer.all.switch", true)
                                } else{
                                    //置灰
                                    selectComBox.enabled = false
                                    saveBtn.enabled = false
                                    resetBtn.enabled = false
                                    content.enabled = false
                                    Presenter.setEQEnable(false)
                                    Presenter.setValueToSettings("equalizer.all.switch", false)
                                }
                            }
                        }
                    }
                }
                Rectangle{
                    id: comBoxArea
                    width: 206
                    height: 36
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    ComboBox {
                        id: selectComBox
                        width: parent.width
                        height: parent.height

                        textRole: "text"
                        iconNameRole: "icon"
                        editable: false
                        enabled: switched
                        currentIndex: curEQ
                        model: comBoxModel
                        onActivated: {
                            if(index > 0) {
                                saveBtn.enabled = false
                                curList = baudsList[index - 1].slice()
                                Presenter.setEQCurMode(index)
                            } else {
                                saveBtn.enabled = true
                                curList = customList
                                Presenter.setEQ(true, 0, customList)
                            }
                            Presenter.setValueToSettings("equalizer.all.curEffect", index)
                        }
                    }
                }
                Rectangle{
                    id: btnsArea
                    width: 178
                    height: parent.height
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        spacing: 10
                        anchors.verticalCenter: parent.verticalCenter
                        Button {
                            id: saveBtn
                            width: 84
                            text: qsTr("Save")
                            enabled: switched && curEQ === 0
                            onClicked: {
                                customList = []
                                customList = curList
                                Presenter.setValueToSettings("equalizer.all.baud_pre", customList[0])
                                Presenter.setValueToSettings("equalizer.all.baud_60", customList[1])
                                Presenter.setValueToSettings("equalizer.all.baud_170", customList[2])
                                Presenter.setValueToSettings("equalizer.all.baud_310", customList[3])
                                Presenter.setValueToSettings("equalizer.all.baud_600", customList[4])
                                Presenter.setValueToSettings("equalizer.all.baud_1K", customList[5])
                                Presenter.setValueToSettings("equalizer.all.baud_3K", customList[6])
                                Presenter.setValueToSettings("equalizer.all.baud_6K", customList[7])
                                Presenter.setValueToSettings("equalizer.all.baud_12K", customList[8])
                                Presenter.setValueToSettings("equalizer.all.baud_14K", customList[9])
                                Presenter.setValueToSettings("equalizer.all.baud_16K", customList[10])

                                Presenter.setValueToSettings("equalizer.all.curEffect", 0)
                            }
                        }
                        Button {
                            id: resetBtn
                            width: 84
                            text: qsTr("Reset")
                            onClicked: {
                                switchBtn.checked = false
                                selectComBox.currentIndex = 1
                                curList = baudsList[0].slice()
                                customList = baudsList[0].slice()
                                Presenter.setValueToSettings("equalizer.all.baud_pre", 12)
                                Presenter.setValueToSettings("equalizer.all.baud_60", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_170", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_310", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_600", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_1K", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_3K", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_6K", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_12K", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_14K", 0)
                                Presenter.setValueToSettings("equalizer.all.baud_16K", 0)

                                Presenter.setValueToSettings("equalizer.all.curEffect", 1)
                                Presenter.setEQEnable(false)
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: content
            width: parent.width
            height: 270
            color: "transparent"
            enabled: switched

            Row {
                width: parent.width
                height: parent.height
                spacing: 10

                Rectangle {
                    id: leftArea
                    width: 72
                    height: parent.height
                    radius: 8
                    color: "#11000000"

                    ColumnLayout {
                        width: parent.width
                        height: parent.height
                        spacing: 10

                        Rectangle {
                            width: parent.width
                            height: 15
                            color: "#00000000"
                            Layout.alignment: Qt.AlignHCenter
                            Loader {
                                property int value: 30

                                id: labelLoader
                                Layout.alignment: Qt.AlignHCenter
                                width: parent.width
                                sourceComponent: Label {
                                    id: label
                                    horizontalAlignment: Text.AlignHCenter
                                    text: preamplifierSlider.value
                                    font: DTK.fontManager.t9
                                }
                                visible: false
                            }
                        }
                        Slider {
                            id: preamplifierSlider
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 20

                            highlightedPassedGroove: true
                            handleType: Slider.HandleType.NoArrowVertical

                            height: 220
                            from: -20
                            to: 20
                            value: curList[0]
                            stepSize: 1
                            orientation: Qt.Vertical

                            onHoveredChanged: {
                                if (preamplifierSlider.hovered == true)
                                    labelLoader.visible = true
                                else
                                    labelLoader.visible = false
                            }
                            onValueChanged: {
                                // update custom data
                                if (pressed) {
                                    selectComBox.currentIndex = 0
                                    curList[0] = value
                                    Presenter.setEQpre(value)
                                    for (var i = 1; i <= curList.length; i++) {
                                        Presenter.setEQbauds(i, curList[i])
                                    }
                                    saveBtn.enabled = true
                                }
                            }
                        }
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.bottomMargin: 10
                            text: qsTr("Preamplifier")
                            font: DTK.fontManager.t9
                        }
                    }
                }
                Rectangle {
                    id: rightArea
                    width: 450
                    height: parent.height
                    radius: 8
                    color: "#11000000"

                    ListView {
                        id: rightSliderListView
                        width: parent.width
                        height: parent.height
                        orientation:  ListView.Horizontal
                        spacing: 4
                        anchors.left: parent.left
                        anchors.leftMargin: 8

                        model: sliderModel
                        delegate: rightSliderDelegate
                    }
                }
            }
        }

        Component {
            id: rightSliderDelegate

            Rectangle {
                id: sliderDelegateRect
                width:40
                height: parent.height
                color: "#00000000"

                ColumnLayout {
                    width: parent.width
                    height: parent.height
                    spacing: 10

                    Rectangle {
                        width: parent.width
                        height: 15
                        color: "#00000000"

                        Loader {
                            id: valueLabelLoader
                            width: parent.width
                            sourceComponent: Label {
                                id: valueLabel
                                horizontalAlignment: Text.AlignHCenter
                                text: delegateSlider.value
                                font: DTK.fontManager.t9
                            }
                            visible: false
                        }
                    }

                    Slider {
                        id: delegateSlider
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 20
                        highlightedPassedGroove: true
                        handleType: Slider.HandleType.NoArrowVertical

                        height: 220
                        from: -20
                        to: 20
                        stepSize: 1
                        value: curList[index + 1]
                        orientation: Qt.Vertical

                        onHoveredChanged: {
                            if (hovered) {
                                valueLabelLoader.visible = true
                            } else {
                                if (!pressed)
                                    valueLabelLoader.visible = false
                            }
                        }
                        onPressedChanged: {
                            if (!pressed && !hovered)
                                valueLabelLoader.visible = false
                        }

                        onValueChanged: {
                            if (pressed) {
                                selectComBox.currentIndex = 0
                                curList[index + 1] = value
                                Presenter.setEQpre(curList[0])
                                for (var i = 1; i <= curList.length; i++) {
                                    Presenter.setEQbauds(i, curList[i])
                                }
                                saveBtn.enabled = true
                            }
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        text: name
                        font: DTK.fontManager.t9
                    }
                }
            }
        }
    }

    onVisibleChanged: {
        dataLoad()

        selectComBox.currentIndex = Qt.binding(function(){return curEQ})

        if (curEQ != 0) {
            Presenter.setEQCurMode(curEQ)
            curList = baudsList[curEQ - 1].slice()
            saveBtn.enabled = false
        } else {
            curList = customList
            Presenter.setEQpre(curList[0])
            for (var i = 1; i < curList.length; i++) {
                Presenter.setEQbauds(i, curList[i])
            }
        }
    }

    function dataLoad(){
        switched = Presenter.valueFromSettings("equalizer.all.switch")
        curEQ = Presenter.valueFromSettings("equalizer.all.curEffect")
        customList = []
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_pre"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_60"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_170"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_310"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_600"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_1K"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_3K"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_6K"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_12K"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_14K"))
        customList.push(Presenter.valueFromSettings("equalizer.all.baud_16K"))
    }

    Component.onCompleted: {
        dataLoad()

        if (curEQ != 0) {
            curList = baudsList[curEQ - 1].slice()
        } else {
            curList = customList
        }
    }
}
