#include "clientSocket.h"

ClientSocket::ClientSocket() {
    gameState = NOT_STARTED;
}

bool ClientSocket::sendMessageToServer(Message message) {
    // Header is empty -> Client have no reponse
    if (message.header == "")
        return true;
    // Otherwise, send the message normally
    string content = message.str();
    if (!MessageTransceiver::send_string(clientSocket, content)) {
        puts("Failed to send message to server");
        return false;
    }
    return true;
}

bool ClientSocket::receiveMessageFromServer(Message &message) {
    puts("Receiving message");
    string content;
    if (!MessageTransceiver::recv_string(clientSocket, content)) {
        return false;
    }
    message = Message(content);
    return true;
}

Message ClientSocket::serverResponseHandler(Message message) {
    cout << "Received message: " << message.str() << endl;

    string header = "";
    vector<string> data;

    if (message.header == HEADER_UNAME_REQUEST || message.header == HEADER_UNAME_INVALID || message.header == HEADER_UNAME_DUPLICATED) {
        if (message.header == HEADER_UNAME_INVALID) {
            puts("Username is invalid");
        }
        else if (message.header == HEADER_UNAME_DUPLICATED) {
            printf("The username '%s' has already been used\n", message.data[0].c_str());
        }
        else {
            puts("Connected to server");
        }

        cout << "Enter your username (not exceeding 10 characters, consist of letters, numbers and '_'): ";
        cin >> username;

        header = HEADER_UNAME_RESPONSE;
        data = {username};
    }
    else if (message.header == HEADER_GAME_ALREADY_STARTED) {
        puts("Game has already started. Please wait for another game.");
    }
    else if (message.header == HEADER_REGISTER_SUCCESS) {
        puts("Registered success! Waiting for the game to start...");
    }
    else if (message.header == HEADER_GAME_START) {
        gameState = ONGOING;

        int keywordLength = stoi(message.data[0]);
        string hint = message.data[1];
        string maskedKeyword = message.data[2];

        puts("Game started!");
        printf("Keyword length: %d\n", keywordLength);
        printf("Masked keyword: %s\n", maskedKeyword.c_str());
        printf("Hint: %s\n", hint.c_str());
    }
    else if (message.header == HEADER_GUESS_CHAR_REQUEST) {
        cout << "Enter the character: ";
        char guessChar;
        cin >> guessChar;

        header = HEADER_GUESS_CHAR_RESPONSE;
        data = {to_string(guessChar)};
    }
    else if (message.header == HEADER_GUESS_CHAR_RESULT) {
        int result = stoi(message.data[0]);
        if (result == 0) {
            printf("Correct! You get another turn.\n");
        }
        else {
            printf("Incorrect! Next player's turn.\n");
        }

//        int score = stoi(message.data[1]);
//        printf("New score: %d\n", score);
    }
    else if (message.header == HEADER_GUESS_KEYWORD_REQUEST) {
        string keyword;
        cout << "Enter the keyword (blank keyword for no guess): ";
        cin >> keyword;

        header = HEADER_GUESS_KEYWORD_RESPONSE;
        data = {keyword};
    }
    else if (message.header == HEADER_GUESS_KEYWORD_RESULT) {
        int result = stoi(message.data[0]);
        if (result == 0) {
            printf("Congratulation! Your keyword is correct!\n");
        }
        else {
            printf("Your keyword is incorrect! Better luck next time...\n");
        }
    }
    else if (message.header == HEADER_UPDATE_GAME_INFO) {
        string maskedKeyword = message.data[0];
        string playerTurn = message.data[1];
        int nClient = stoi(message.data[2]);

        printf("Keyword: %s\n", maskedKeyword.c_str());

        for(int i = 3; i < message.data.size(); i += 3) {
            string username = message.data[i];
            int playerScore = stoi(message.data[i+1]);
            bool isDisqualified = stoi(message.data[i+2]);

            printf("Player: %s - Score: %d - ", username.c_str(), playerScore);
            puts(isDisqualified ? "Disqualified" : "In game");
        }
    }
    else if (message.header == HEADER_GAME_FINISH) {
        gameState = FINISHED;
        puts("Game finished");

        string keyword = message.data[0];
        int nClient = stoi(message.data[1]);
        printf("Keyword: %s\n", keyword.c_str());

        for(int i = 2; i < message.data.size(); i += 2) {
            string username = message.data[i];
            int playerScore = stoi(message.data[i+1]);
            printf("Player: %s - Score: %d\n", username.c_str(), playerScore);
        }
    }
    else if (message.header == HEADER_BAD_MESSAGE) {
        puts("Something wrong I can feel it...");
        exit(1);
    }
    else { // Unknown syntax -> Bad syntax
        header = HEADER_BAD_MESSAGE;
    }

    return Message(header, data);
}

void ClientSocket::serverDisconnectedHandler() {

}


bool ClientSocket::initSocket() {
  if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    puts("Socket creation error");
    return false;
  }

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    puts("Invalid address/ Address not supported");
    return false;
  }

  if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    puts("Connection Failed");
    return false;
  }

  return true;
}

void ClientSocket::mainLoop() {
  fd_set rfds;
  int retval, maxfd = -1;

  while (true) {
    // Clear the collection of readable file descriptors
    FD_ZERO(&rfds);
    // Add standard input file descriptors to the collection
    FD_SET(0, &rfds);
    // Add the currently connected fd to the collection
    FD_SET(clientSocket, &rfds);

    if (maxfd < clientSocket) maxfd = clientSocket;

    // Setting the timeout
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

//    puts("Selecting");

    // Waiting for chat from the server
    retval = select(maxfd+1, &rfds, NULL, NULL, &tv);

//    puts("Selected");

    if (retval == -1) {
      perror("Select failed");
      exit(1);
    } else if (retval == 0) {
      continue;
    } else {
      if (FD_ISSET(clientSocket, &rfds)) {
        // Receive message from the server
        Message serverMessage;
        // Check if it was for closing
        if (!receiveMessageFromServer(serverMessage)) {
            serverDisconnectedHandler();
            puts("Connection to server closed");
            break;
        }

        Message clientMesssage = serverResponseHandler(serverMessage);
        sendMessageToServer(clientMesssage);
      }
    }

    if (gameState == FINISHED) {
        break;
    }
  }

  close(clientSocket);
}
