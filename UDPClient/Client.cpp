#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET _socket;
int color;
string nickname;

DWORD WINAPI Sender(void* param)
{
    bool isFirstConnection = true;
    while (true) {
        string message;
        if (isFirstConnection) {
            message = "First connection:" + nickname + '(' + to_string(color) + ')';
            isFirstConnection = false;
        }
        else {
            string query;
            cin >> query;
            message = query + ':' + nickname + '(' + to_string(color) + ')';
        }
        send(_socket, message.c_str(), message.length(), 0);
    }
}

void ReceiveChatHistory() {
    char response[DEFAULT_BUFLEN];

    int result = recv(_socket, response, DEFAULT_BUFLEN, 0);
    if (!result) {
        return;
    }
    response[result] = '\0';

    cout << "Chat history received:\n" << response << endl;
}


DWORD WINAPI Receiver(void* param)
{
    while (true) {
        char response[DEFAULT_BUFLEN];
        int result = recv(_socket, response,DEFAULT_BUFLEN, 0);
        response[result] = '\0';

        istringstream iss(response);
        string message_text, nickname_str, color_str;

        getline(iss, message_text, ':');
        getline(iss, nickname_str, '(');
        getline(iss, color_str, ')');
        int local_color = stoi(color_str);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, local_color);

        cout << message_text << ": " << nickname_str<<endl;

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    }
}


BOOL ExitHandler(DWORD whatHappening)
{
    switch (whatHappening)
    {
    case CTRL_C_EVENT: // closing console by ctrl + c
    case CTRL_BREAK_EVENT: // ctrl + break
    case CTRL_CLOSE_EVENT: // closing the console window by X button
        return(TRUE);
        break;
    default:
        return FALSE;
    }
}

int main()
{
    // обработчик закрытия окна консоли
    //SetConsoleCtrlHandler((PHANDLER_ROUTINE)ExitHandler, true);

    system("title Client");

    // initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // разрешить адрес сервера и порт
    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    // пытаться подключиться к адресу, пока не удастся
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // создать сокет на стороне клиента для подключения к серверу
        _socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }

        // connect to server
        iResult = connect(_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(_socket);
            _socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }
    cout << "Nickname:" << endl;
    cin >> nickname;
    cout << "Color:" << endl;
    cin >> color;
    CreateThread(0, 0, Sender, 0, 0, 0);
    ReceiveChatHistory();
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
} 
