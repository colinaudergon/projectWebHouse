function openConnection() {
  // const url = `ws://${ip}:${port}`;
  socket = new WebSocket ('ws://127.0.0.1:8000');

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
}

function send() {
  var textField  = document.getElementById("DataToSendID");
  socket.send(textField.value);
}