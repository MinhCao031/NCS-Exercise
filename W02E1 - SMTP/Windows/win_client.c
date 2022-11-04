#include <stdio.h>
#include <strings.h>
#include <winsock2.h>

#define MAX_BYTE 1048576

// Put here the IP address of the server
const char szServerIPAddr[20] = "192.168.12.51";
//const char szServerIPAddr[20] = "192.168.12.108"

// The server port that will be used by clients to talk with the server
const int nServerPort = 25; 

BOOL InitWinSock2_0() {
  WSADATA wsaData;
  WORD wVersion = MAKEWORD(2, 0);
  if (!WSAStartup(wVersion, &wsaData))
    return TRUE;
  return FALSE;
}

int main(/*int argc, char* argv[]*/) {
  if (!InitWinSock2_0()) {
    printf("Unable to Initialize Windows Socket environment, ERRORCODE = %d\n",
      WSAGetLastError());
    return -11;
  }

  SOCKET hClientSocket;
  hClientSocket = socket(
    AF_INET,    // The address family. AF_INET specifies TCP/IP
    SOCK_STREAM,// Protocol type. SOCK_STREM specified TCP
    0           // Protoco Name. Should be 0 for AF_INET address family
  );

  if (hClientSocket == INVALID_SOCKET) {
    printf("Unable to create Server socket\n");
    // Cleanup the environment initialized by WSAStartup()
    WSACleanup();
    return -12;
  }

  //print some info
  printf("Server's IP Address: %s\n", szServerIPAddr);
  printf("Server's port:   %d\n", nServerPort);
  printf("Socket ID:       %d\n", hClientSocket);

  // Create the structure describing various Server parameters
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET; // The address family. MUST be AF_INET
  serverAddr.sin_addr.s_addr = inet_addr(szServerIPAddr);
  serverAddr.sin_port = htons(nServerPort);

  // Connect to the server
  if (connect(hClientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
    printf("Unable to connect to %s on port %d\n", szServerIPAddr, nServerPort);
    closesocket(hClientSocket);
    WSACleanup();
    return -13;
  }

  printf("Connection established ............\n\n");
  char *strRecv = malloc(MAX_BYTE);
  int check = -1;

  // Step 1: First signal from hmail server
  recv(hClientSocket, strRecv, MAX_BYTE, 0);
  printf("Received: %s\n", strRecv);
  if (strstr(strRecv, "220") == NULL) {
    return 220;
  }

  for (int i = 1; i < 100; i++) {
    // Input command
    printf("Loop %d, input: ", i);
    fgets(strRecv, MAX_BYTE, stdin);
    strRecv[strlen(strRecv)-1] = '\0';
    // strRecv[strlen(strRecv)-1] = '\r';
    // sprintf(strRecv, "%s\n", strRecv);

    printf("You typed: $$$%s&&&\n", strRecv);
    check = send(hClientSocket, strRecv, MAX_BYTE, 0);

    //printf("check send: %d\n", check);
    //printf("Sent: %s\n", strRecv);
    char data[MAX_BYTE];
    check = recv(hClientSocket, data, MAX_BYTE, 0);
    //printf("check recv: %d\n", check);
    printf("Received: %s\n", data);
    for (int j = 0; j < MAX_BYTE; j++) {
      data[j] = '\0';
      strRecv[i] = '\0';
    }
  }

  closesocket(hClientSocket);
  WSACleanup();
  return 0;
}
