## POSIX : 
  Portable OS interface. its a fam of standards defined by IEEE that ensures compatibility between UNIX like OS
  it gives functions like socket, bind, listen, accept ,pthread_create 
  ### This meaning that this server code can run on any POSIX-compliant OS with little or no change

## Threads (pthreads, multithreading)

  Definition: A thread is like a mini-program running inside your process. POSIX Threads (pthreads) is the API for creating/managing threads.
  Functions used:
  pthread_create() → start a new thread.
  pthread_detach() → free resources when the thread ends.
 ## Each client runs in its own thread (client_thread()), so multiple clients can connect simultaneously without blocking each other.

## Socket API :
  a socket is an endpoint for communication betwen two machines , the socket API provides functions 
  like socket , binf ,listen,accept,send, recv " to create and use sockets" 
  ### this server uses sockets to listen for and accept client connections, like a telephone operator handling calls

## Socket() : 
  ```
  socket(AF_INET, SOCK_STREAM, 0);
  ```
  this simply create s asocket 
  AF-INET -> for IPV4 addr
  SOCK_STREAM -> TCP protocol 
  0 -> its the default protocol for TCP 
  ### this returns a file descriptor(int) that represents socket , so basically creats the server that is listening 

## bind() :
  ```
  bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  ```
  this is responsible for attaching a socket to a IP and port
  here the server_fd is the int value form prev socket() function 
  address is struct with IP AND PORT 
  ### ensures that the server isl istening 

## listen()
  ```
  listen(server_fd, 10);
  ```
  Marks the socket as passive and ready to accept incoming connections.
  server_fd → The listening socket.
  10 → Backlog queue (max number of pending connections).
  ### Starts waiting for clients like a call center putting itself "open for calls."

## accept()
  ```
  accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
  ```
  Accepts a new connection from a client.
  Return: A new socket descriptor for that client (different from server_fd).
  ### Creates a dedicated "phone line" for each connected client, so server_fd keeps listening while the new socket handles the conversation.

## inet_addr()
  ```
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  ```
  Definition: Converts a human-readable IP string (127.0.0.1) into a 32-bit binary value in network order.
  ### Tells the OS which IP the server will listen on.

## Understang of how the program works : 
  socket() → "Buy a telephone"

  The server gets a phone (socket) but it’s not yet connected.
  
  setsockopt() → "Set phone settings"
  
  Adjust settings so the line can be reused quickly.
  
  bind() → "Register phone number"
  
  The server says: “I will answer calls on number (IP:port).”
  
  listen() → "Turn ringer on"
  
  The server is now waiting for calls (clients).
  
  accept() → "Pick up the phone"
  
  When a client calls, the server picks up and gets a new phone line just for that client.
  
  The old phone (server socket) keeps ringing for new calls.
  
  pthread_create() → "Hire a new operator"
  
  Each client call is handed to a new thread (operator).
  
  That operator will chat with the client independently.
  
  Client thread (send/recv) → "Talk on the line"
  
  The operator exchanges messages with the client.
  
  Example: Server reads request → sends sensor data back.
  
  pthread_detach() → "Operator cleans up"
  
  When conversation ends, the operator goes home (frees resources).
  
  signal(SIGPIPE, SIG_IGN) → "Don’t panic if someone hangs up suddenly"
  
  If a client disconnects mid-conversation, the server ignores it instead of crashing.

## Diagram 
```
           +---------------------+
           |   socket()          |
           |   (create phone)    |
           +---------------------+
                     |
           +---------------------+
           | setsockopt()        |
           | (set options)       |
           +---------------------+
                     |
           +---------------------+
           |   bind()            |
           | (assign IP:Port)    |
           +---------------------+
                     |
           +---------------------+
           |   listen()          |
           | (wait for calls)    |
           +---------------------+
                     |
     --------------------------------
     |                              |
+-----------------+          +-----------------+
| accept()        |          | accept()        |
| (answer client) |   ...    | (answer client) |
+-----------------+          +-----------------+
         |                           |
+-----------------+          +-----------------+
| pthread_create()|          | pthread_create()|
| (new operator)  |          | (new operator)  |
+-----------------+          +-----------------+
         |                           |
   +-----------+                +-----------+
   | talk loop |                | talk loop |
   | (send/recv)|                | (send/recv)|
   +-----------+                +-----------+

```
