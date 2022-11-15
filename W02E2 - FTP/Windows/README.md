- To compile client
```c
gcc client.c -o democlient.exe -lWs2_32
```

- To run client
```c
.\democlient.exe [userName] [password] [serverAddress] [fileName] [serverPort]
```

- Example of running client to connect to Doanh's server
```c
.\democlient.exe doanh doanh 192.168.12.65 data.txt 21
```
