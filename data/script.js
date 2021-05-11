  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
  }
  
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
 
  function onMessage(event) { 
    switch(event.data) {
      case '0': document.getElementById("state1").innerHTML = "OFF"; break
      case '1': document.getElementById("state1").innerHTML = "ON"; break
      case '2': document.getElementById("state2").innerHTML = "OFF"; break
      case '3': document.getElementById("state2").innerHTML = "ON"; break
      case '4': document.getElementById("state3").innerHTML = "OFF"; break
      case '5': document.getElementById("state3").innerHTML = "ON"; break
    }
    } 
  
  function onLoad(event) {initWebSocket();initButton();}

  function initButton() {
    document.getElementById('button1').addEventListener('click', toggle1);
	document.getElementById('button2').addEventListener('click', toggle2);
	document.getElementById('button3').addEventListener('click', toggle3);
  }
  
  function toggle1(){websocket.send('toggle1');}
  function toggle2(){websocket.send('toggle2');}
  function toggle3(){websocket.send('toggle3');}