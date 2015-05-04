import QtQuick 2.1

Item{
	property bool disabled: false
	property bool hoverEnabled: true
	property url normal_image
	property url hover_image
	property url pressed_image
	property url disabled_image

	signal hovered
	signal clicked
	signal pressed
	signal released
	signal exited

	state: 'normal'

	MouseArea {
	    anchors.fill: parent
	    hoverEnabled: parent.hoverEnabled
	    onEntered: {
	    	parent.hovered();
	    	parent.state = "hovered"
	    }
	    onExited: {
	    	parent.exited()
	    	parent.state = "normal"
	    }

	    onPressed:{
	    	parent.pressed()
	    	parent.state = "pressed"
	    }

	    onReleased:{
	    	parent.released()
	    	parent.state = "normal"
	    }

	    onClicked: {
	    	parent.clicked()
	    }
	}

	Image {
        id: image
        sourceSize.width: parent.width
        sourceSize.height: parent.height
    }

	states:[
		State{
			name: "normal"
			PropertyChanges {target: image; source: normal_image}
		},
		State{
			name: "hovered"
			PropertyChanges {target: image; source: hover_image}
		},
		State{
			name: "pressed"
			PropertyChanges {target: image; source: pressed_image}
		},
		State{
			name: "disabled"
			PropertyChanges {target: image; source: disabled_image}
		}
	]
}
