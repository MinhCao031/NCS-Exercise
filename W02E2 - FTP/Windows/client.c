// CLIENT
#include <stdio.h>
#include <strings.h>
#include <winsock2.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE_1 512
#define SIZE_2 32768

char *instruction = "\nExpected format: .\\[compile].exe [1] [2] [3] [4] [5]\n"
    "[1]: 0 is to type any, 1 is auto-send, 2 is auto-receive\n"
    "[2]: Server's address\n"
    "[3]: Your userName\n"
    "[4]: Your password\n"
    "[5]: File's name\n\n";
char *userName1;
char *password1;
char *serverIPAddr;
char *fileName;
int serverPort = 21;
int dataPort;
int sendORrecv; // 1 is sent, 2 is recv
int temp;

SOCKET clientSocket;

void handleArguments(
    int* sendORrecv, char** userName1, char** password1, 
    char** serverIPAddr, char** fileName, int argc, char* argv[]
);

BOOL InitWinSock2_0();

int setupAndListen (SOCKET *socketFromClient, char* IPAddr, int port);

void sendToServer (SOCKET toSocket, char* cmd1, char* cmd2);

char* recvFromServer (SOCKET fromSocket);

char* sendAndRecv (SOCKET toSocket, char* send, char* recv);

void calculatePasvPort(char* message, int* pasvPort);

BOOL sentFile(SOCKET dataSocket, char* fileName);

BOOL recvFile(SOCKET dataSocket, char* fileName);

int main (int argc, char* argv[]) {
    printf("%s", instruction);
    handleArguments(
        &sendORrecv, &userName1, &password1, 
        &serverIPAddr, &fileName, argc, argv
    );

    // Init socket
    if (!InitWinSock2_0()) {
        printf("Unable to Initialize Windows Socket environment, ERRORCODE = %d\n",
        WSAGetLastError());
        return -11;
    }
    temp = setupAndListen(&clientSocket, serverIPAddr, serverPort);
    if (temp < 0) {
        printf("ERROR #1 temp = %d", temp);
        return -100;
    }


    if (sendORrecv > 0) {
    // STEP 0: make sure there is a connection
        recvFromServer(clientSocket);
    // STEP 1: sent username
        sendAndRecv(clientSocket, "USER ", userName1);
    // STEP 2: sent password
        sendAndRecv(clientSocket, "PASS ", password1);   
    // STEP 3: set directory
        sendAndRecv(clientSocket, "CWD ", "/");   
    // STEP 4: sent type A
        sendAndRecv(clientSocket, "TYPE ", "A");   
    // SEND FILE
        if (sendORrecv == 1) {
        // STEP 5.0: try entering Passive Mode
            char *dataPortInfo = sendAndRecv(clientSocket, "PASV", "");;
        // STEP 5.1: calculate data Port
            calculatePasvPort(dataPortInfo, &dataPort);
        // STEP 6: specify file name to sent to server
            sendAndRecv(clientSocket, "STOR ", fileName);
        // STEP 7.0: setup another socket for data transfer
            SOCKET dataSocket;
            temp = setupAndListen(&dataSocket, serverIPAddr, dataPort);
            if (temp < 0) {
                printf("ERROR #2 temp = %d", temp);
                return -201;
            }    
            printf("Ready to send files ............\n\n");
        // STEP 7.1: sent file
            if(!sentFile(dataSocket, fileName)) {
                printf("ERROR #3");
                return -202;
            } else closesocket(dataSocket);
    // RECEIVE FILE       
        } else if (sendORrecv == 2) {
        // STEP 5.0: try entering Passive Mode
            char *dataPortInfo = sendAndRecv(clientSocket, "PASV", "");;
        // STEP 5.1: calculate data Port
            calculatePasvPort(dataPortInfo, &dataPort);
        // STEP 6: specify file name to sent to server
            sendAndRecv(clientSocket, "RETR ", fileName);
        // STEP 7.0: setup another socket for data transfer
            SOCKET dataSocket;
            temp = setupAndListen(&dataSocket, serverIPAddr, dataPort);
            if (temp < 0) {
                printf("ERROR #4 temp = %d", temp);
                return -301;
            }    
            printf("Ready to receive files ............\n\n");
        // STEP 7.1: sent file
            if(!recvFile(dataSocket, fileName)) {
                printf("ERROR #5");
                return -302;
            } else closesocket(dataSocket);        
        } else {
            printf("\n*****\nERROR: The second parameter must be 0, 1 or 2\n*****\n");
        }
    // STEP 8: quit
        sendAndRecv(clientSocket, "QUIT", "");
        closesocket(clientSocket);        
    } else {
    // STEP 0: make sure there is a connection
        recvFromServer(clientSocket);

        while (1) {
            char* inp = malloc(SIZE_1);
            printf("\n\nPlease type a command: ");
            fgets(inp, SIZE_1, stdin);
            *(inp + strlen(inp) - 1) = '\0';
            // printf("Sent: ->%s<-\n\n", inp);
            char *out = sendAndRecv(clientSocket, inp, "");
            if (strstr(inp, "PASV")) {
                calculatePasvPort(out, &dataPort);
                continue;
            } else if (strstr(inp, "STOR")) {
                SOCKET dataSocket;
                temp = setupAndListen(&dataSocket, serverIPAddr, dataPort);
                if (temp < 0) {
                    printf("ERROR #6 temp = %d", temp);
                    return -401;
                }
                fileName = inp+5;
                if (sentFile(dataSocket, fileName)) {
                    printf("Sent successfully\n");
                    closesocket(dataSocket); 
                } else {
                    printf("ERROR #7");
                    return -402;
                }        
            } else if (strstr(inp, "RETR")) {
                SOCKET dataSocket;
                temp = setupAndListen(&dataSocket, serverIPAddr, dataPort);
                if (temp < 0) {
                    printf("ERROR #6 temp = %d", temp);
                    return -401;
                }
                fileName = inp+5;
                if (recvFile(dataSocket, fileName)) {
                    printf("Receive successfully\n");
                    closesocket(dataSocket); 
                } else {
                    printf("ERROR #8");
                    return -403;
                } 
            } else if (strstr(inp, "QUIT")) break;
        }
    }

    WSACleanup();
    printf("%s", instruction);
    return 0;
}

/** ---------------------------------------- End of main function ---------------------------------------- **/

BOOL InitWinSock2_0 () {
    WSADATA wsaData;
    WORD wVersion = MAKEWORD(2, 0);
    if (!WSAStartup(wVersion, &wsaData))
        return TRUE;
    return FALSE;
}

void handleArguments (
    int* sendORrecv, char** userName1, char** password1, 
    char** serverIPAddr, char** fileName, int argc, char* argv[]
) {
    switch(argc) {
        case 6: *fileName = argv[5];
        case 5: *password1 = argv[4];
        case 4: *userName1 = argv[3];
        case 3: *serverIPAddr = argv[2];
        case 2: {
            *sendORrecv = atoi(argv[1]);
            if (*sendORrecv == 0) *serverIPAddr = "127.0.0.1";
            break;
        }
        default: {
            *userName1 = "m01";
            *password1 = "1111";
            *sendORrecv = 1;
            *fileName = "rndvid.mp4";
            break;
        }
    }
    printf("userName: %s\npassword: %s\nserverIPAddr: %s\nfileName: %s\n",
        *userName1, *password1, *serverIPAddr, *fileName);
}

int setupAndListen(SOCKET* socketFromClient, char* IPAddr, int port) {
    *socketFromClient = socket(
        AF_INET,     // The address family. AF_INET specifies TCP/IP
        SOCK_STREAM, // Protocol type. SOCK_STREM specified TCP
        0 // Protoco Name. Should be 0 for AF_INET address family
    );
    printf("Client Socket ID: %d\n", *socketFromClient);
    if (*socketFromClient == INVALID_SOCKET) {
        printf("Unable to create Server socket\n");
        // Cleanup the environment initialized by WSAStartup()
        WSACleanup();
        return -12;
    }

    // Create the structure describing various Server parameters
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // The address family. MUST be AF_INET
    serverAddr.sin_addr.s_addr = inet_addr(IPAddr);
    serverAddr.sin_port = htons(port);

    // Connect to the server
    if (connect(*socketFromClient, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Unable to connect to %s on port %d\n", IPAddr, port);
        closesocket(*socketFromClient);
        WSACleanup();
        return -13;
    }
    printf("Connection established ............\n\n");
    return (int) *socketFromClient;
}

void sendToServer(SOCKET toSocket, char* cmd1, char* cmd2) {
    char *toSend = malloc(SIZE_1);
    sprintf(toSend, "%s%s\r\n", cmd1, cmd2);
    send(toSocket, toSend, strlen(toSend), 0);
    printf("Sent: {%s}\n", toSend);    
}

char* recvFromServer(SOCKET fromSocket) {
    char *toRecv = malloc(SIZE_1);
    recv(fromSocket, toRecv, SIZE_1, 0);
    printf("Received: [%s]\n", toRecv);
    return toRecv;
}

char* sendAndRecv(SOCKET socketID, char* cmd1, char* cmd2) {
    sendToServer(socketID, cmd1, cmd2);
    return recvFromServer(socketID);
}

void calculatePasvPort(char* message, int* pasvPort) {
    int countComma = 0;
    int num[4] = {0, 0, 0, 0};
    for (int i = 0; *(message + i) != ')'; i++) if (*(message + i) == ',') {
        // printf("i: %d\n", i);
        countComma++;
        if (countComma > 3) num[countComma - 2] = i + 1;
    }
    num[0] = atoi(message + num[2]);
    num[1] = atoi(message + num[3]);
    // printf("%d\t%d\n", num[2], num[3]);
    // printf("%d\t%d\n", num[0], num[1]);
    *pasvPort =  num[0] * 256 + num[1];
    printf("Data Port = %d * 256 + %d = %d\n\n", num[0], num[1], *pasvPort);
}

BOOL sentFile(SOCKET dataSocket, char* fileName) {
    FILE *fileSend;
    fileSend = fopen (fileName, "rb");
    if(!fileSend) {
        printf ("Error while reading the file\n");
        return FALSE;
    } else {
        //printf ("File opened successfully!\n");
        printf("Client data has connected!");
        fseek(fileSend, 0, SEEK_END);
        unsigned long Size = ftell(fileSend);
        fseek(fileSend, 0, SEEK_SET);
        char *buffer = malloc(Size);

        printf("\n%d\n", Size);
        for (int i = Size; i >= 0; i -= SIZE_2) {
            fread(buffer, min(i,SIZE_2), 1, fileSend);
            int sentSize = send(dataSocket, buffer, min(i,SIZE_2), 0);
            printf("7. Sent: [%d bytes], %d bytes left\n\n",
                sentSize, (i - SIZE_2 > 0? i - SIZE_2: 0));
            printf("7. Sent: {%s}\n\n\n", buffer);
        }

        fclose(fileSend);
        free(buffer);
        return TRUE;
        //printf ("File sent successfully!\n");
    }
}

BOOL recvFile(SOCKET dataSocket, char* fileName) {
    FILE *fileRecv;
    fileRecv = fopen (fileName, "wb");
    if(!fileRecv) {
        printf ("Error while reading the file\n");
        return FALSE;
    } else {
        //printf ("File opened successfully!\n");
        char* buffer = malloc(SIZE_2);
        int recvBytes = recv(dataSocket, buffer, SIZE_2, 0);
        while (recvBytes > 0) {
            fwrite(buffer, sizeof(buffer[0]), recvBytes, fileRecv);
            recvBytes = recv(dataSocket, buffer, SIZE_2, 0);
        }
        fclose(fileRecv);
        return TRUE;
        //printf ("File sent successfully!\n");
    }
}
