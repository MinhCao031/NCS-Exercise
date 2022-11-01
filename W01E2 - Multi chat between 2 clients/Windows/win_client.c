#include <winsock2.h>
#include <stdio.h>

char szServerIPAddr[ 20 ] = "127.0.0.1" ;   // Put here the IP address of the server
int nServerPort = 8081 ;                    // The server port that will be used by                                            // clients to talk with the server

void* doReceiving(void* sockID);
BOOL InitWinSock2_0() ;

int main()
{
    //printf("Enter the server IP Address: ");
    //scanf("%s", szServerIPAddr);
    //printf("Enter the server port number: ");
    //scanf("%d", nServerPort);
    printf("\n%s\t%d\n", szServerIPAddr, nServerPort);

    if ( ! InitWinSock2_0() ) {
        printf("Unable to Initialize Windows Socket environment, ERRORCODE = %d\n", WSAGetLastError() );
        return -11 ;
    }

    SOCKET hClientSocket;
    hClientSocket = socket(
        AF_INET,        // The address family. AF_INET specifies TCP/IP
        SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
        0               // Protoco Name. Should be 0 for AF_INET address family
    );
    printf("%d\n", hClientSocket);
    if ( hClientSocket == INVALID_SOCKET ) {
        printf("Unable to create Server socket\n");
        // Cleanup the environment initialized by WSAStartup()
        WSACleanup( ) ;
        return -12 ;
    }

    // Create the structure describing various Server parameters
    struct sockaddr_in serverAddr ;

    serverAddr.sin_family = AF_INET ;     // The address family. MUST be AF_INET
    serverAddr.sin_addr . s_addr = inet_addr( szServerIPAddr ) ;
    serverAddr.sin_port = htons( nServerPort ) ;

    // Connect to the server
    if ( connect( hClientSocket, ( struct sockaddr * ) &serverAddr, sizeof( serverAddr ) ) < 0 ) {
        printf("Unable to connect to %s on port %d\n", szServerIPAddr, nServerPort);
        closesocket( hClientSocket ) ;
        WSACleanup() ;
        return -13 ;
    }

    HANDLE chatThread;
    DWORD dwThreadId;
    chatThread = CreateThread(NULL, 0,
        (void *) doReceiving,
        (void *) &hClientSocket,
        0, &dwThreadId
    );

    printf("Connection established ............\n");

    BOOL closed = FALSE;
    while ( 0==0 ) {
        char *szBuffer = malloc(1024);
		scanf("%s", szBuffer);

		if(strcmp(szBuffer,"LIST") == 0){
			send(hClientSocket,szBuffer,1024,0);
		}
		if(strcmp(szBuffer,"SEND") == 0){
			send(hClientSocket,szBuffer,1024,0);
			scanf("%s",szBuffer);
			send(hClientSocket,szBuffer,1024,0);
			scanf("%[^\n]s",szBuffer);
			while(*szBuffer == ' ') szBuffer++;
			send(hClientSocket,szBuffer,1024,0);
		}
		if(strcmp(szBuffer,"QUIT") == 0) {
            closed = TRUE;
            send(hClientSocket,"QUIT",1024,0);
            break;
		}
    }
    if (!closed) send(hClientSocket,"QUIT",1024,0);
    closesocket(hClientSocket);
    WSACleanup() ;
    return 0 ;
}

BOOL InitWinSock2_0()
{
    WSADATA wsaData ;
    WORD wVersion = MAKEWORD( 2, 0 ) ;
    if ( ! WSAStartup( wVersion, &wsaData ) )
        return TRUE ;
    return FALSE ;
}


void* doReceiving(void* hClientSocket){
	int clientSocket = *((int*) hClientSocket);

	while(1){
		char data[1024];
		int read = recv(clientSocket,data,1024,0);
		data[read] = '\0';
		printf("%s\n",data);
		for (int k = 0; k < 1024; k++) data[k] = '\0';
	}
}
