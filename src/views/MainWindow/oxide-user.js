console.log('oxide:', oxide);

oxide.addMessageHandler('runJavascript', function(msg){
	var newEvent = new CustomEvent("OxideRunJavascript", {detail: msg.args.detail});
	document.dispatchEvent(newEvent);
});
