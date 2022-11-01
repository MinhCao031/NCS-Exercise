#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CLIENT 8

struct CLIENT_INFO {
    int index;      // Index of client, used as parameter when sending to another client
    int id;         // Id of client, used to identify client
    BOOL existed;   // Status of client, true if it's currently active, otherwise false
    SOCKET hClientSocket;
    struct sockaddr_in clientAddr;
};

struct CLIENT_INFO client[MAX_CLIENT];

// Put here the IP address of the server
char szServerIPAddr[ ] = "192.168.12.140";

// The server port that will be used by clients to talk with the server
int nServerPort = 8081;

// The number of clients that is always up-to-date
int clientCount = 0;
int serverloop = 0;

BOOL InitWinSock2_0();
char* concat(char* left, char* right);
BOOL WINAPI ClientThread(LPVOID lpData);
BOOL WINAPI ServerThread(LPVOID lpData);

int main() {
    // Initialize Window Sockets
    if(!InitWinSock2_0()) {
        printf("Unable to Initialize Windows Socket environment%d\n", WSAGetLastError());
        return -1;
    }

    SOCKET hServerSocket;
    hServerSocket = socket (
        AF_INET,        // The address family. AF_INET specifies TCP/IP
        SOCK_STREAM,    // Protocol type. SOCK_STREAM specified TCP
        0               // Protocol Name. Should be 0 for AF_INET address family
    );

    // Create server socket
    if(hServerSocket == INVALID_SOCKET) {
        printf("Unable to create Server socket\n");
        // Cleanup the environment initialized by WSAStartup()
        WSACleanup();
        return -1;
    }

    // Create the structure describing various Server parameters
    struct sockaddr_in serverAddr;
    // The address family. MUST be AF_INET
    serverAddr.sin_family = AF_INET;
    // The IP address
    serverAddr.sin_addr.s_addr = inet_addr(szServerIPAddr);
    // The server's port
    serverAddr.sin_port = htons(nServerPort);

    // Bind the Server socket to the address & port
    if(bind(hServerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Unable to bind to %s port %d\n", szServerIPAddr, nServerPort);
        // Free the socket and cleanup the environment initialized by WSAStartup()
        closesocket(hServerSocket);
        WSACleanup();
        return -2;
    }

    // Put the Server socket in listen state so that it can wait for client connections
    if(listen(hServerSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Unable to put server in listen state\n");
        // Free the socket and cleanup the environment initialized by WSAStartup()
        closesocket(hServerSocket);
        WSACleanup();
        return -3;
    }

    printf("Server started listening on port %d ............\n", nServerPort);
    DWORD dwServerThreadId;
    HANDLE hServerThread = CreateThread (
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) ServerThread,
        (LPVOID) &clientCount,
        0,
        &dwServerThreadId
    );

    //int ru = 0;
    // Start the infinite loop
    while (clientCount < MAX_CLIENT) { // Limit to 30 clients
        // For accept()'s parameter that needs a pointer
        int nSize = sizeof(client[clientCount].clientAddr);
        //ru = ru+1;
        //printf("$%d ",ru);

        // As the socket is in listen mode there is a connection request pending.
        // Calling accept() will succeed and return the socket for the request.
        client[clientCount].hClientSocket = accept (
            hServerSocket,
            (struct sockaddr*) &client[clientCount].clientAddr,
            &nSize
        );

        // Check if a client is connected or not
        if(client[clientCount].hClientSocket == INVALID_SOCKET) {
            printf("ERROR: accept() function failed\n");
        } else {
            HANDLE hClientThread;   // Variable that create/store a thread
            DWORD dwThreadId;       // Just for CreateThread()'s parameter

            // Store some info about the newly connected client
            client[clientCount].index   = clientCount;
            client[clientCount].id      = client[clientCount].hClientSocket;
            client[clientCount].existed = TRUE;

            // Start the client thread
            hClientThread = CreateThread (
                NULL,
                0,
                (LPTHREAD_START_ROUTINE) ClientThread,
                (LPVOID) &client[clientCount],
                0,
                &dwThreadId
            );

            // Add to total clients
            clientCount = 1+clientCount;

            // Error
            if(hClientThread == NULL) {
                printf("Unable to create client thread\n");
            } else {
                CloseHandle(hClientThread);
            }
        }
    }
    // Server shutting down
    closesocket(hServerSocket);
    WSACleanup();
    return 0;
}

BOOL InitWinSock2_0() {
    WSADATA wsaData;
    WORD wVersion = MAKEWORD(2, 0);
    if(!WSAStartup(wVersion, &wsaData))
        return TRUE;
    return FALSE;
}

//concatenate 2 string
char* concat(char* left, char* right) {
    if (!left) return right;
    int i,j,k=0;
    char *ans = malloc(255);
    for (i = 0; left[i] != '\0'; i++, k++) {
        ans[k] = left[i];
    }
    for (j = 0; right[j] != '\0'; j++, k++) {
        ans[k] = right[j];
    }
    ans[k] = '\0';
    return ans;
}

BOOL WINAPI ServerThread(LPVOID lpData) {
    while(1) {
        int numOfOnl = 0;
        char *inp = malloc(1024);
        scanf("%s", inp);
        if(strcmp(inp,"LIST") == 0) {
            // Check for inactive/lost clients
            for(int i = 0 ; i < clientCount ; i ++) {
                int test = send(client[i].hClientSocket,"",1024,0);
                if (test < 0) client[i].existed = FALSE;
            }
            //List active clients
            printf("\nALL CLIENT BELOW:\n");
            for(int i = 0 ; i < clientCount ; i ++)
                if(client[i].existed) {
                    printf("Client %d is at socket %d: online.\n",i + 1,(int)client[i].hClientSocket);
                    numOfOnl += 1;
                } else {
                    printf("Client %d is at socket %d: offline.\n",i + 1,(int)client[i].hClientSocket);
                }
            printf("TOTAL ONLINE: %d\n", numOfOnl);
        }
        free(inp);
    }

}

BOOL WINAPI ClientThread(LPVOID lpData) {
    struct CLIENT_INFO* clientDetail = (struct CLIENT_INFO*) lpData;
	int index = clientDetail->index;
	int clientSocket = (int)clientDetail->hClientSocket;
	printf("Client %d connected.\n",index + 1);

	while(1){
        // Request from server: List all active clients
		char data[1024] = "";
		int read = recv(clientSocket,data,1024,0);

		// printf("$%d ", serverloop);
		serverloop += 1;
		if (read >= 0) data[read] = '\0';

		char *output = malloc(1024);
		// Request from client: List all active clients
		if(strcmp(data,"LIST") == 0){
			int l = 0;
			// Check for inactive/lost clients
			for(int i = 0 ; i < clientCount ; i ++) {
                int test = send(client[i].hClientSocket,"   ",1024,0);
                if (test < 0) client[i].existed = FALSE;
			}
			//List active clients
			for(int i = 0 ; i < clientCount ; i ++)
                if(client[i].existed && i != index) {
                    l += snprintf(output + l,1024,
                        "Client %d is at socket %d.\n",
                        i + 1,(int)client[i].hClientSocket
                    );
                }
			send(clientSocket,output,1024,0);
			continue;
		}
		else if(strcmp(data,"SEND") == 0) {
			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';
			int id = atoi(data) - 1;
			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';
			//send(client[id].hClientSocket,data,1024,0);

			int clientSender = 0;
			//Find who send the message
            for (int i = 0; i <= clientCount; i++) {
                if (client[i].existed && (int)client[i].hClientSocket > 0
                    && clientSocket == (int)client[i].hClientSocket) {
                    clientSender = i;
                    break;
                }
			}
			char sender[18] = "FROM CLIENT 00: ";
			sender[12] = (clientSender+1)/10 + 48;
			sender[13] = (clientSender+1)%10 + 48;
			char* out = concat(sender,data);
			int status = send(client[id].hClientSocket,out,1024,0);
			// printf("%d\t%s\nSTATUS: %d\n", clientSender, out, status);
			if(status < 0)
                send(client[clientSender].hClientSocket,"ERROR",1024,0);

		}
		else if(strcmp(data,"QUIT") == 0) {
			read = recv(clientSocket,data,1024,0);
			// printf("%d\t%d\t%s\n", read, clientSocket, data);
			// Find who went offline correctly
			for (int i = 0; i < clientCount; i++) {
                if (clientSocket == (int)client[i].hClientSocket) {
                    client[i].existed = FALSE;
                    printf("Found exited client: %d\n", i+1);
                    break;
                }
			}
		}

        for (int k = 0; k < 1024; k++) {
            data[k] = '\0';
        }
        free(output);
	}
	return TRUE;
}
