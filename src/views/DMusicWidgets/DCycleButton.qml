import QtQuick 2.3
import DMusic 1.0

DIconButton{

	id: root
	property int playbackMode: 3

	property var oneLoop_normal_image: '../../skin/icons/dark/appbar.grade.a.png'
    property var oneLoop_hover_image: '../../skin/icons/dark/appbar.grade.a.png'
    property var oneLoop_pressed_image: '../../skin/icons/light/appbar.grade.a.png'
    property var oneLoop_disabled_image: '../../skin/icons/dark/appbar.grade.a.png'

    property var sequentialLoop_normal_image: '../../skin/icons/dark/appbar.grade.b.png'
    property var sequentialLoop_hover_image: '../../skin/icons/dark/appbar.grade.b.png'
    property var sequentialLoop_pressed_image: '../../skin/icons/light/appbar.grade.b.png'
    property var sequentialLoop_disabled_image: '../../skin/icons/dark/appbar.grade.b.png'

    property var random_normal_image: '../../skin/icons/dark/appbar.grade.c.png'
    property var random_hover_image: '../../skin/icons/dark/appbar.grade.c.png'
    property var random_pressed_image: '../../skin/icons/light/appbar.grade.c.png'
    property var random_disabled_image: '../../skin/icons/dark/appbar.grade.c.png'


    function getNormalByMode(playbackMode) {
    	var image;
    	if (playbackMode == 1){
    		image = oneLoop_normal_image;
    	}else if (playbackMode == 3){
    		image = sequentialLoop_normal_image;
		}else if (playbackMode == 4){
			image = random_normal_image;
		}else{
            image = sequentialLoop_normal_image;
        }
        return image;
    }

    function getHoverByMode(playbackMode) {
    	var image;
    	if (playbackMode == 1){
    		image = oneLoop_hover_image;
    	}else if (playbackMode == 3){
    		image = sequentialLoop_hover_image;
		}else if (playbackMode == 4){
			image = random_hover_image;
		}else{
            image = sequentialLoop_normal_image;
        }
        return image;
    }

    function getPressedByMode(playbackMode) {
    	var image;
    	if (playbackMode == 1){
    		image = oneLoop_pressed_image;
    	}else if (playbackMode == 3){
    		image = sequentialLoop_pressed_image;
		}else if (playbackMode == 4){
			image = random_pressed_image;
		}else{
            image = sequentialLoop_normal_image;
        }
        return image;
    }

    function getDisabledByMode(playbackMode) {
    	var image;
    	if (playbackMode == 1){
    		image = oneLoop_disabled_image;
    	}else if (playbackMode == 3){
    		image = sequentialLoop_disabled_image;
		}else if (playbackMode == 4){
			image = random_disabled_image;
		}else{
            image = sequentialLoop_normal_image;
        }
        return image;
    }

    normal_image: getNormalByMode(root.playbackMode)
    hover_image: getHoverByMode(root.playbackMode)
    pressed_image: getPressedByMode(root.playbackMode)
    disabled_image: getDisabledByMode(root.playbackMode)

    onClicked:{
    	if (root.playbackMode == 3){
    		root.playbackMode = 4;
    	}else if (playbackMode == 4){
    		root.playbackMode = 1;
    	}else if (playbackMode == 1){
    		root.playbackMode = 3;
    	}
    }
}