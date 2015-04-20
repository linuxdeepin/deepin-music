oxide.addMessageHandler('runJavaScript', function(msg){
    var newEvent = new CustomEvent("OxideRunJavascript", {detail: msg.args.detail});
    document.dispatchEvent(newEvent);
});
