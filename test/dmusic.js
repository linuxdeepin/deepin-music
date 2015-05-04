(function(global, undefined){
var Dmusic = {};

function play(id, type){
    var message = JSON.stringify({
        'rpcVersion': '1.0', 
        'rpcId':id, 
        'rpcType': type
    })
    console.error(message);
}


function enqueue(id, type){
    var message = JSON.stringify({
        'rpcVersion': '1.0', 
        'rpcId':id, 
        'rpcType': type
    })
    console.error(message);
}


function download(id, type){
    var message = JSON.stringify({
        'rpcVersion': '1.0', 
        'rpcId':id, 
        'rpcType': type
    })
    console.error(message);
}


function addFavorite(id){

    var message = JSON.stringify({
        'rpcVersion': '1.0', 
        'rpcId':id,
    })
    console.error(message);

    return { success: true };
}

function removeFavorite(id){
     var message = JSON.stringify({
        'rpcVersion': '1.0', 
        'rpcId':id,
    })
    console.error(message);

    return { success: true };
}


Dmusic.play = play;

Dmusic.enqueue = enqueue;

Dmusic.download = download;

Dmusic.addFavorite = addFavorite;

Dmusic.removeFavorite = removeFavorite;

global.Dmusic = Dmusic;
})(window);