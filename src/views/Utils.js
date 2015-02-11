
function loadqml(qmlfile, parent, properties) {

    var instance;
    var component;
    function finishCreation() {
        if (component.status == Component.Ready) {
            instance = component.createObject(parent, properties);
            if (instance == null) {
                // Error Handling
                console.log("Error creating object");
            }
            parent.push(instance);
        } else if (component.status == Component.Error) {
            // Error Handling
            console.log("Error loading component:", component.errorString());
        }
    }

    component = Qt.createComponent(qmlfile);
    if (component.status == Component.Ready){
        instance = component.createObject(parent, properties);
        parent.push(instance);
    }
    else
        component.statusChanged.connect(finishCreation);
}


function resetSkin() {
    playBottomBar.color = "#282F3F"
    bgIamge.source = ''
}


function setSkinByImage(url) {
    if (url === undefined){
        url = "../skin/images/bg2.jpg"
    }
    playBottomBar.color = "transparent"
    bgIamge.source = url
}


function playToggle(playing){
    if (playing){
        player.play()
        if (player.mediaObject)
           player.mediaObject.notifyInterval = 50;
    }else{
        player.pause()
    }
}


function playMusic(musicID){
    var url = PyUtil.getMusicURLByID(musicID);
    player.stop()
    player.source = url;
    playToggle(true)
}


function onPlaying(){
    playBottomBar.playing = true;
}

function onPaused(){
    playBottomBar.playing = false;
}

function onStopped(){
    playBottomBar.playing = false;
}

function onError(error, errorString){
    playBottomBar.playing = false;
    print(error, errorString);
}
