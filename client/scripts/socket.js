
function openConnection() {
  const ip = document.getElementById("ServerIP").value;
  const port=8000;
  const url = `ws://${ip}:${port}`;
  // socket = new WebSocket ('ws://192.168.178.80:8000');
  socket = new WebSocket (url);

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
    console.log("Received something");
    var textField  = document.getElementById("ReceivedDataID");
    textField.value = message.data;
  }
}

function closeConnection () {
  socket.close();
  console.log("Disconnected");
}

function send() {
  var textField  = document.getElementById("DataToSendID");
  console.log(textField.value);
  // var textField  = "{cmd:R,dev:TV,val:0}";
  socket.send(textField.value);
}