(function(global, undefined){
var Dmusic = {};

var moment = require('moment');

Dmusic.mkString = function(obj) {
    if (Object.prototype.toString.call(obj) === '[object Array]') {
        return obj.toString();
    } else {
        return obj.toString();
    }
};

Dmusic.debugOutput = function(message){
    console.info('[DEBUG ' + moment().format('YYYY/M/D HH:mm:ss') + '] ' + message);
};

function debugFunctor(actionType) {
    return function(id, type) {
        this.debugOutput('To ' + actionType + ' ' + type + ': ' + this.mkString(id));
    };
}

function debugFavoriteFunctor(actionType) {
    return function(id) {
        this.debugOutput(actionType + ' favorite for: ' + this.mkString(id));
        return { success: true };
    };
}

Dmusic.play = debugFunctor('play');

Dmusic.enqueue = debugFunctor('enqueue');

Dmusic.download = debugFunctor('download');

Dmusic.addFavorite = debugFavoriteFunctor('Add');

Dmusic.removeFavorite = debugFavoriteFunctor('Remove');

global.Dmusic = Dmusic;
})(window);