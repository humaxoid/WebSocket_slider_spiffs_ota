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
  
  function onOpen(event) { console.log('Connection opened'); }
  function onClose(event) { console.log('Connection closed'); setTimeout(initWebSocket, 2000); }
 
  function onMessage(event) {
    var state1; if (event.data == "1"){ state1 = "ON";
    // document.getElementById('button1').style.backgroundColor = "#04b50a";
    } else { state1 = "OFF"; // document.getElementById('button1').style.backgroundColor = "#c90411";
    }
      document.getElementById('state1').innerHTML = state1;
//====
   var state2; if (event.data == "3"){ state2 = "ON";} else {state2 = "OFF";}
     document.getElementById('state2').innerHTML = state2;
  }
 
  function onLoad(event) { initWebSocket(); initButton(); }

  function initButton() {
    document.getElementById('button1').addEventListener('click', toggle1);
    document.getElementById('button2').addEventListener('click', toggle2);
   }
   
  function toggle1(){ websocket.send('toggle1'); }
  function toggle2(){ websocket.send('toggle2'); }
