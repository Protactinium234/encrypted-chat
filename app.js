// TODO: Bold currently selected user
// TODO: Actually add chat controls. Currently there's only one chat box for everyone

let username = '';
let socket = null;

// Login function
function login() {
    username = document.getElementById('username').value;
    if (username.trim() !== '') {
        document.getElementById('login').style.display = 'none';
        document.getElementById('chat').style.display = 'block';
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
        console.log(event.data);
        switch (message.type) {
            case 'userList':
                updateUserList(message.users);
                break;
            case 'chat':
                displayMessage(message);
                break;
            case 'error':
                displayError(message.error);
                break;
            default:
                displayError('Server side error :(');
                break;

        }
    };

    socket.onclose = () => {
        const message = 'Disconnected from server'
        console.log(message);
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
    const userList = document.getElementById('userList');
    userList.innerHTML = '';
    users.forEach((user) => {
        if (user !== username) {
            const li = document.createElement('li');
            li.textContent = user;
            li.onclick = () => startChat(user);
            userList.appendChild(li);
        }
    });
}

// Start chat with a user
function startChat(user) {
    document.getElementById('chatWindow').innerHTML = '';
    document.getElementById('messageInput').dataset.receiver = user;
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
        displayMessage(chatMessage);
        messageInput.value = '';
    }
}

// Display message in chat window
function displayMessage(message) {
    document.getElementById('errorBox').style.display = 'none';
    const chatWindow = document.getElementById('chatWindow');
    const p = document.createElement('p');
    p.textContent = `${message.sender}: ${message.message}`;
    chatWindow.appendChild(p);
    chatWindow.scrollTop = chatWindow.scrollHeight;
}