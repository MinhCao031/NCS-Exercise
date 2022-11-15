// CLIENT
#include <stdio.h>
#include <strings.h>
#include <winsock2.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_SIZE 1024
#define MAX_BYTE_READ 1048576

BOOL InitWinSock2_0();

int main(int argc, char* argv[]) {
    printf("Expected format: .\\[compile].exe [userName] [password] [serverAddress] [fileName] [serverPort]\n");
    char *userName1 = malloc(64);
    char *password1 = malloc(32);
    char *serverIPAddr = malloc(20);
    int serverPort;
    int dataPort;
    char *fileName = malloc(256);
    switch(argc) {
        case 6: serverPort = atoi(argv[5]);
        case 5: fileName = argv[4];
        case 4: serverIPAddr = argv[3];
        case 3: password1 = argv[2];
        case 2: {
            userName1 = argv[1];
            break;
        }
        default: {
            userName1 = "m01";
            password1 = "1111";
            serverIPAddr = "127.0.0.1";
            serverPort = 21;
            fileName = "data.txt";
            // fileName = "2022-11-03.png";
            // dataPort = 204*256+47;
            break;
        }
    }
    printf("userName: %s\npassword: %s\nserverIPAddr: %s\nserverPort: %d\nfileName: %s\n",
        userName1, password1, serverIPAddr, serverPort, fileName);

    // Init socket
    if (!InitWinSock2_0()) {
        printf("Unable to Initialize Windows Socket environment, ERRORCODE = %d\n",
        WSAGetLastError());
        return -11;
    }
    SOCKET clientSocket;
    clientSocket = socket(
        AF_INET,     // The address family. AF_INET specifies TCP/IP
        SOCK_STREAM, // Protocol type. SOCK_STREM specified TCP
        0 // Protoco Name. Should be 0 for AF_INET address family
    );
    printf("Client Socket ID: %d\n", clientSocket);
    if (clientSocket == INVALID_SOCKET) {
        printf("Unable to create Server socket\n");
        // Cleanup the environment initialized by WSAStartup()
        WSACleanup();
        return -12;
    }

    // Create the structure describing various Server parameters
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // The address family. MUST be AF_INET
    serverAddr.sin_addr.s_addr = inet_addr(serverIPAddr);
    serverAddr.sin_port = htons(serverPort);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Unable to connect to %s on port %d\n", serverIPAddr, serverPort);
        closesocket(clientSocket);
        WSACleanup();
        return -13;
    }

    printf("Connection established ............\n\n");

    // STEP 0: make sure there is a connection
    char *welcome = malloc(MAX_SIZE);
    recv(clientSocket, welcome, MAX_SIZE, 0);
    printf("0. Received: %s\n", welcome);

    // STEP 1: sent username
    char *userinfo = malloc(MAX_SIZE);
    sprintf(userinfo, "USER %s\r\n", userName1);
    send(clientSocket, userinfo, strlen(userinfo), 0);
    printf("1. Sent: %s\n", userinfo);

    char *verifyusr = malloc(MAX_SIZE);
    recv(clientSocket, verifyusr, MAX_SIZE, 0);
    printf("1. Received: %s\n", verifyusr);

    // STEP 2: sent password
    char *passinfo = malloc(MAX_SIZE);
    sprintf(passinfo, "PASS %s\r\n", password1);
    send(clientSocket, passinfo, strlen(passinfo), 0);
    printf("2. Sent: %s\n", passinfo);

    char *verifypwd = malloc(MAX_SIZE);
    recv(clientSocket, verifypwd, MAX_SIZE, 0);
    printf("2. Received: %s\n", verifypwd);

    // STEP 3: set directory
    send(clientSocket, "CWD /\r\n", strlen("CWD /\r\n"), 0);
    printf("3. Sent: %s\n", "CWD /\r\n");

    char *cwd = malloc(MAX_SIZE);
    recv(clientSocket, cwd, MAX_SIZE, 0);
    printf("3. Received: %s\n", cwd);

    // STEP 4: sent type A
    send(clientSocket, "TYPE A\r\n", strlen("TYPE A\r\n"), 0);
    printf("4. Sent: %s\n", "TYPE A\r\n");

    char *typeA = malloc(MAX_SIZE);
    recv(clientSocket, typeA, MAX_SIZE, 0);
    printf("4. Received: %s\n", typeA);

    // STEP 5.0: try entering Passive Mode
    send(clientSocket, "PASV\r\n", strlen("PASV\r\n"), 0);
    printf("5. Sent: %s\n", "PASV\r\n");

    char *dataPortInfo = malloc(MAX_SIZE);
    recv(clientSocket, dataPortInfo, MAX_SIZE, 0);
    printf("5. Received: %s\n", dataPortInfo);

    // STEP 5.1: calculate data Port
    int countComma = 0;
    int num[4] = {0, 0, 0, 0};
    for (int i = 0; *(dataPortInfo + i) != ')'; i++) if (*(dataPortInfo + i) == ',') {
        // printf("i: %d\n", i);
        countComma++;
        if (countComma > 3) num[countComma - 2] = i + 1;
    }
    num[0] = atoi(dataPortInfo+num[2]);
    num[1] = atoi(dataPortInfo+num[3]);
    // printf("%d\t%d\n", num[2], num[3]);
    // printf("%d\t%d\n", num[0], num[1]);   
    dataPort =  num[0] * 256 + num[1];
    printf("Data Port = %d * 256 + %d = %d\n\n", num[0], num[1], dataPort);

    // STEP 6: specify file name
    char *sentFile = malloc(MAX_SIZE);
    sprintf(sentFile, "STOR %s\r\n", fileName);
    send(clientSocket, sentFile, strlen(sentFile), 0);
    printf("6. Sent: %s\n", sentFile);

    char *startSending = malloc(MAX_SIZE);
    recv(clientSocket, startSending, MAX_SIZE, 0);
    printf("6. Received: %s\n", startSending);

    // STEP 7.0: setup another socket for data transfer
    SOCKET dataSocket;
    dataSocket = socket(
        AF_INET,     // The address family. AF_INET specifies TCP/IP
        SOCK_STREAM, // Protocol type. SOCK_STREM specified TCP
        0 // Protoco Name. Should be 0 for AF_INET address family
    );
    printf("Data Socket ID: %d\n", dataSocket);
    if (dataSocket == INVALID_SOCKET) {
        printf("Unable to create Server socket\n");
        // Cleanup the environment initialized by WSAStartup()
        WSACleanup();
        return -14;
    }

    // Create the structure describing various Server parameters
    struct sockaddr_in dataServerAddr;
    dataServerAddr.sin_family = AF_INET; // The address family. MUST be AF_INET
    dataServerAddr.sin_addr.s_addr = inet_addr(serverIPAddr);
    dataServerAddr.sin_port = htons(dataPort);

    // Connect to the server
    if (connect(dataSocket, (struct sockaddr *)&dataServerAddr, sizeof(dataServerAddr)) < 0) {
        printf("Unable to connect to %s on port %d\n", serverIPAddr, dataPort);
        closesocket(dataSocket);
        WSACleanup();
        return -15;
    }

    printf("Another connection established, ready to send files ............\n\n");


    // STEP 7.1: sent file
    FILE *File;
    File = fopen (fileName, "rb");
    if(!File) {
        printf ("Error while reading the file\n");
    } else {
        // printf ("File opened successfully!\n");
        fseek(File, 0, SEEK_END);
        unsigned long Size = ftell(File);
        fseek(File, 0, SEEK_SET);
        char *Buffer = malloc(Size);
        
        for (int i = Size; i >= 0; i -= MAX_BYTE_READ) { 
            fread(Buffer, MAX_BYTE_READ, 1, File);
            int sentSize = send(dataSocket, Buffer, MAX_BYTE_READ, 0);
            printf("7. Sent: [%d bytes], %d bytes left\n\n",
                sentSize, (i - MAX_BYTE_READ > 0? i - MAX_BYTE_READ: 0));
        }
        
        fclose(File);        
        free(Buffer);
        // printf ("File sent successfully!\n");
    }
    closesocket(dataSocket);


    // STEP 8: quit
    send(clientSocket, "QUIT\r\n", strlen("QUIT\r\n"), 0);
    printf("8. Sent: %s\n", "QUIT\r\n");

    char *confirmExit = malloc(MAX_SIZE);
    recv(clientSocket, confirmExit, MAX_SIZE, 0);
    printf("8. Received: %s\n", confirmExit);

    closesocket(clientSocket);
    WSACleanup();
    printf("Expected format: .\\[compile].exe [userName] [password] [serverAddress] [fileName] [serverPort]\n");
    return 0;
}

BOOL InitWinSock2_0() {
  WSADATA wsaData;
  WORD wVersion = MAKEWORD(2, 0);
  if (!WSAStartup(wVersion, &wsaData))
    return TRUE;
  return FALSE;
}
