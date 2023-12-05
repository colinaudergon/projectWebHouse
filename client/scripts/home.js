function callInitSocket() {
    // Get the values of ipTxt and portTxt
    var ipValue = document.getElementById("ipTxt").value;
    var portValue = document.getElementById("portTxt").value;
    const socket = window.openConnection(ipValue, portValue);
}


// on modal load get current socket ip, port, and displays it
function getCurrentSocket(){
    // const currentSocket = window.getWebSocket();
    // console.log("Current socket: ",currentSocket);
    console.log("got current socket!")

}


