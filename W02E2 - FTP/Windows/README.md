- To compile client
```c
gcc client.c -o democlient.exe -lWs2_32
```

- To run client (with at most 6 parameters)
  - x = 0: Feel free to type any command (Only need 3 parameters)

  - x = 1: File is sent automatically (Need 6 parameters)

  - x = 2: File is received automatically (Need 6 parameters)
```c
.\democlient.exe [x] [serverAddress] [userName] [password] [fileName]
```
- 3 examples of running client
```c
.\democlient.exe 1 192.168.12.65 doanh doanh data.txt
.\democlient.exe 2 192.168.12.99 minh 1111 vid.mp4
.\democlient.exe 0 127.0.0.1
```
