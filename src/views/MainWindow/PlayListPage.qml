import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
	id: root

    Row {

    	anchors.topMargin: 10
    	anchors.leftMargin: 14
    	anchors.fill: parent

    	Rectangle {
    		id: playlistNameBar
    		width : 120
    		height: root.height
    		color: "white"

    		Column {

    			
    			anchors.fill: parent

    			Rectangle {
    				id: fixPlaylistBar
    				width: 120
    				height: 64

    				Column {
    					spacing: 16

    					Row {
    						spacing: 10
    						DStarButton {
    							id: starButton
    							width: 16
    							height: 16
    						}

    						Text {
    							width: fixPlaylistBar.width - starButton.width
    							height: 16
    							color: "#868686"
    							font.pixelSize: 12
    							verticalAlignment: Text.AlignVCenter
    							text: '我的收藏'
    						}
    					}

    					Row {
    						spacing: 10

    						DStarButton {
    							id: wavButton
    							width: 16
    							height: 16
    						}

    						Text {
    							width: fixPlaylistBar.width - wavButton.width
    							height: 16
    							color: "#868686"
    							font.pixelSize: 12
    							verticalAlignment: Text.AlignVCenter
    							text: '试听歌单'
    						}

    					}
    				}

    			}

    			Rectangle {
    				id: separator
    				width: 120
    				height: 1
    				color: "#dfdfdf"
    			}

    			Rectangle {
    				width: 120
    				height: playlistNameBar.height - separator.height - fixPlaylistBar.height
    				color: "white"
    			}
    			
    		}
    	}

    	Rectangle {
    		width: root.width - playlistNameBar.width
    		height: root.height
    		color: "#145278"
    	}

    }
}