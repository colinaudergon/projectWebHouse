
function openConnection() {
  // const url = `ws://${ip}:${port}`;
  socket = new WebSocket ('ws://192.168.178.80:8000');

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
  // var textField  = document.getElementById("DataToSendID");
  var textField  = "{cmd:R/W,dev:TV/RL/SL/L1/L2/HE/TE/AA,val:0..99}";
  socket.send(textField.value);
}