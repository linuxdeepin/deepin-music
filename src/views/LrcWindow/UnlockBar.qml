import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0
import "../DMusicWidgets/LrcWindow"

Rectangle {
	id: unLockedBar
	height: 50
    width : 436
	anchors.fill: parent
	anchors.topMargin: 18
	color: "transparent"
	visible: !locked
	Row {
	    id: layout
	    anchors.fill: parent
	    spacing: 14
	    Row {
	        id: playLayout
	        spacing: 10
	        Rectangle {
	            width: toolBar.normalHeight
	            height: toolBar.largerHeight
	            color: 'transparent'
	            DLrcPreButton{
	                anchors.centerIn: parent
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	                onClicked:{
	                    print('++++++++++')
	                }
	            }


	        }

	        DLrcPlayButton {
	            width: toolBar.largerHeight
	            height: toolBar.largerHeight
	        }
	        Rectangle {
	            width: toolBar.normalHeight
	            height: toolBar.largerHeight
	            color: 'transparent'
	            DLrcNextButton{
	                anchors.centerIn: parent
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }
	        }
	    }

	    Rectangle {

	        width: layout.width - playLayout.width - 14
	        height:toolBar.largerHeight
	        color: "transparent"
	        Row {
	            height: toolBar.normalHeight
	            anchors.verticalCenter: parent.verticalCenter
	            spacing: 14
	            DLrcFontPlusButton {
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcFontMinusButton {
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcBackButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcForwordButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcThemeButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }
	            DLrcSingleLineButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcDoubleLineButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcKalaokButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }

	            DLrcLockButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight

	                onClicked:{
	                    unLockedBar.visible = false;
	                    toolBar.locked = true;
	                    SignalManager.locked();
	                }
	            }

	            DLrcSettingButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }
	            DLrcSearchButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }
	            DLrcCloseButton{
	                width: toolBar.normalHeight
	                height: toolBar.normalHeight
	            }
	        }
	    }
	}
	}