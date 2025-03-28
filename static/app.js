let username = '';
let socket = null;
let active = '';

let chatBoxes = new Object();
let liList = new Object();
let unreadCount = new Object();
let chatWindowClone = document.getElementById('chatWindow').cloneNode(true);
let chatBoxDefault = chatWindowClone.cloneNode(true);
const userList = document.getElementById('userList');
let para = document.createElement('p');
para.textContent = "Select a User to Start Chatting";
para.style.textAlign = "center";
chatBoxDefault.appendChild(para);
document.getElementById('chatWindow').replaceWith(chatBoxDefault);

// Login function
function login() {
    username = document.getElementById('username').value;
    if (username.trim() !== '') {
        document.getElementById('login').style.display = 'none';
        document.getElementById('chat').style.display = 'grid';
        initSocket();
    }
}

// Initialize WebSocket connection
function initSocket() {
    socket = new WebSocket('ws://localhost:8080');
    
    socket.onopen = () => {
        console.log('Connected to server');
        socket.send(JSON.stringify({ type: 'login', username: username }));
    };

    socket.onmessage = (event) => {
        const message = JSON.parse(event.data);
        // console.log(event.data);
        switch (message.type) {
            case 'userList':
                updateUserList(message.users);
                break;
            case 'chat':
                displayMessage(message.sender, message, false);
                break;
            case 'error':
                displayError(message.error);
                break;
            case 'prime':
                generateKey(user);
            default:
                displayError('Server side error :(');
                break;

        }
    };

    socket.onclose = () => {
        const message = 'Disconnected from server'
        // console.log(message);
        displayError(message)
    };

    socket.onerror = (error) => {
        console.error('WebSocket error: ', error);
    };
}

// Display error message
function displayError(message) {
    const errorBox = document.getElementById('errorBox');
    errorBox.textContent = message;
    errorBox.style.display = 'block';
    document.getElementById('login').style.display = 'block';
    document.getElementById('chat').style.display = 'none';
}


// Update the user list
function updateUserList(users) {
    document.getElementById('errorBox').style.display = 'none';
    // If user no longer exists
    Object.keys(liList).forEach((user) => {
        if (!users.includes(user)) {
            userList.removeChild(liList[user]);
            delete liList[user];
            delete chatBoxes[user];
            delete unreadCount[user];
            if (active == user) {
                active = '';
                document.getElementById('chatWindow').replaceWith(chatBoxDefault);
            }
        }
    });
    users.forEach((user) => {
        if (user !== username) {
            if (chatBoxes[user] === undefined) {
                chatBoxes[user] = chatWindowClone.cloneNode(true);
                let p = document.createElement('p');
                p.textContent = "Start of your chat with " + user;
                p.style.textAlign = "center";
                chatBoxes[user].appendChild(p);
            }
            if (liList[user] === undefined) {
                unreadCount[user] = 0;
                liList[user] = document.createElement('li');
                liList[user].innerHTML = "<span class=\"user-name\">" + user + "</span>";
                liList[user].onclick = () => startChat(user);
                userList.appendChild(liList[user]);
            }
        }
    });
}

// Start chat with a user
function startChat(user) {
    document.getElementById('messageInput').dataset.receiver = user;
    liList[user].style.fontWeight = 'normal';
    liList[user].innerHTML = "<span class=\"user-name\">" + user + "</span>";
    unreadCount[user] = 0;
    document.getElementById('chatWindow').replaceWith(chatBoxes[user]);
    active = user;
}

// Send a message
function sendMessage() {
    const messageInput = document.getElementById('messageInput');
    const message = messageInput.value;
    const receiver = messageInput.dataset.receiver;
    if (message.trim() !== '' && receiver) {
        const chatMessage = {
            type: 'chat',
            sender: username,
            receiver: receiver,
            message: message
        };
        socket.send(JSON.stringify(chatMessage));
        displayMessage(chatMessage.receiver, chatMessage, true);
        messageInput.value = '';
    }
}

// Display message in chat window
function displayMessage(user, message, show) {
    document.getElementById('errorBox').style.display = 'none';
    const chatWindow = chatBoxes[user];
    const p = document.createElement('p');
    p.textContent = `${message.sender}: ${message.message}`;
    chatWindow.appendChild(p);
    chatWindow.scrollTop = chatWindow.scrollHeight;
    userList.insertBefore(liList[user],userList.firstChild);
    if (show) {
        document.getElementById('chatWindow').replaceWith(chatWindow);
        active = user;
    }
    else if (active != user) {
        liList[user].style.fontWeight = "bold";
        liList[user].innerHTML = `
        <span class="user-name">` + user + `</span>
        <span class="unread-count">` + ++unreadCount[user] + `</span>`;
    }
}