import QtQuick 2.4

Item {
	property var rootWindow
	property var catgoryCombox
	property var musicManagerLoader

	function updateWindow(index) {
	    if (index == 0){
    		musicManagerLoader.source = './ArtistPage.qml';
    	}else if(index == 1){
    		musicManagerLoader.source = './AlbumPage.qml';
		}else if (index == 2){
			musicManagerLoader.source = './SongPage.qml';
		}else if (index == 3){
			musicManagerLoader.source = './FolderPage.qml';
		}
	}

	Connections {
	    target: catgoryCombox
	    onCurrentIndexChanged: {
	    	var currentIndex = catgoryCombox.currentIndex;
	    	updateWindow(currentIndex);
	    }

	    onItemClicked: {
	    	var currentIndex = catgoryCombox.currentIndex;
	    	catgoryCombox.visible = false
	    }
	}

	Connections {
	    target: musicManagerLoader
	    onLoaded: {
	    	musicManagerLoader.focus = true;
	    } 
	}


	Connections {
	    target: rootWindow
	    onVisibleChanged:{
	    	if (visible){
	    		updateWindow(catgoryCombox.currentIndex);
	    	}
	    } 
	}
}