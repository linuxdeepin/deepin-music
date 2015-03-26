import QtQuick 2.4
import DMusic 1.0

Rectangle {
    id: playlistDetailBox

    property var playlistView: playlistView

    Rectangle {
    	id: detailBox
    	anchors.fill: parent
    	anchors.leftMargin: 25
    	anchors.rightMargin: 35
    	anchors.bottomMargin: 20

    	Column {
    		anchors.fill: detailBox

    		spacing: 5

    		Rectangle {
			    id: header

			    // property var playButton: playButton

			    width: parent.width
			    height: 24
			    color: "transparent"

			    Row {

			        anchors.fill: parent
			        spacing: 10

			        Rectangle{
			            id: tipRect
			            width: 20
			            height: 24
			        }

			        Row {
			            height: 24
			            spacing: 38

			            Text {
			                id: titleTetx
			                width: 250
			                height: 24
			                color: "#3a3a3a"
			                font.pixelSize: 12
			                elide: Text.ElideRight
			                // verticalAlignment: Text.AlignVCenter
			                text: '12212121'
			            }

			            Text {
			                id: artistText
			                width: 156
			                height: 24
			                color: "#8a8a8a"
			                font.pixelSize: 12
			                elide: Text.ElideRight
			                // verticalAlignment: Text.AlignVCenter
			                text: '1212121'
			            }

			            Text {
			                id: durationText
			                width: 100
			                height: 24
			                color: "#8a8a8a"
			                font.pixelSize: 12
			                elide: Text.ElideRight
			                // verticalAlignment: Text.AlignVCenter
			                text: '23322332'
			            }
			        }
			    }
			}

    		ListView {
				id: playlistView
	    		clip: true
	    		width: parent.width
	            height: detailBox.height - header.height
	            highlightMoveDuration: 1
	            delegate: MusicDelegate {}
	            focus: true
	            spacing: 14
	            displayMarginBeginning: -12
	            displayMarginEnd: -12
	            snapMode:ListView.SnapToItem
	    	}
    	}

		
    }
}