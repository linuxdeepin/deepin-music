import QtQuick 2.4

Rectangle {
	id: root

    Row {
    	
    	Rectangle {
    		id: playlistNameBar
    		width : 134
    		height: root.height
    		color: "white"

    		Column {
    			
    		}
    	}

    	Rectangle {
    		width: root.width - playlistNameBar.width
    		height: root.height
    		color: "#145278"
    	}

    }
}