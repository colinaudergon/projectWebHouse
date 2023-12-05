// JavaScript functions popup
let isModalOpen = false;

function openPopup(id) {
    document.getElementById(id).style.display = "block";
    isModalOpen = true;
}

// function closePopup(id) {
//     document.getElementById(id).style.display = "none";
//     isModalOpen = false;
// }

function closePopup(id) {
    var popup = document.getElementById(id);

    if (popup) {
        popup.style.display = "none";
        isModalOpen = false;
    }
}

// Close the popup if the user clicks outside the modal
window.onclick = function (event) {
    var editId = document.getElementById("edit");
    var addDeviceId = document.getElementById("addDevice");
    var socketId = document.getElementById("chooseSocket");
    switch(event.target){
        case(editId):
            closePopup('edit');
        break;
        case(addDeviceId):
            closePopup('addDevice');
        break;
        case(socketId):
            closePopup('chooseSocket');
        break;
    }
};
//Test the esc key, if esc is pressed, closes the popup
document.addEventListener('DOMContentLoaded', function () {
    document.addEventListener('keydown', function (event) {
      if (isModalOpen && event.key === 'Escape') {
        closePopup('edit');
        closePopup('addDevice');
        closePopup('chooseSocket');
      }
    });
  });