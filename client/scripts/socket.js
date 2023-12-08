// Assuming you have a WebSocket server running at ws://your-websocket-server
//Must work on this part

let socket = null;

// 'ws://192.168.2.100:8000'



function openConnection() {

  // const url = `ws://${ip}:${port}`;
  socket = new WebSocket("ws://192.168.178.83:8000");
  // if (ip !== null || port !== null){
  //   socket = new WebSocket("ws://192.168.178.83:8000");
  // }
  // else{
  //   socket = new WebSocket (url);
  // }

  socket.onopen = function () {
    alert("connection has been established");
  }

  socket.onerror = function () {
    alert("connection has not been established");
  }

  socket.onclose = function () {
    alert("connection has been closed");
  }

  socket.onmessage = function (message) {
    var textField  = document.getElementById("ReceivedDataID");
    textField.value = message.data;
  }
}

function closeConnection () {
  socket.close();
  clearInterval(interval);
}

//value: <id:X,state:X,val:X>
function send(value) {
  socket.send(value);
}


// socket.onmessage = function (message) {
//   var textField  = document.getElementById("ReceivedDataID");
//   textField.value = message.data;
// }


function getWebSocket(){
  var state;
  switch(socket.readyState){
    case 0:
      state = "connecting";
      break;
    case 1:
      state = "open";
      break;
    case 2:
      state = "closing";
      break;
    case 3:
      state = "closed";
      break;
  }
  return state
}