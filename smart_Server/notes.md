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

# HOW TO BUILD A SERVER ???

When we say “building a web server”, we are essentially creating a program that:

Listens for connections from clients (like browsers, Postman, or even an ESP32).

Understands what the client is asking for (the HTTP request).

Decides what to do based on that request.

Sends a proper response back (in HTML, JSON, plain text, etc.).

## 1. Headers (<unistd.h>, <arpa/inet.h>)

These headers give us access to system-level networking functions:

close() to properly shut sockets (cleanup).

<arpa/inet.h> for IP-related functions (inet_addr, htons, etc.), which help convert between human-readable IP/ports and what the network stack actually uses.

Without them, your program wouldn’t know how to “speak TCP/IP.”

📌 Why needed? Because HTTP runs on top of TCP/IP, so our server must manage sockets at the system level.


## 2. send_response (Build + send HTTP headers + body)

HTTP requires structured responses (status line, headers, then body). Example:

HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 42

{"temperature": 28.4, "led": "on"}


You can’t just send “28.4” or “on/off” — the client wouldn’t understand.

📌 Why needed? This ensures browsers, Postman, or other IoT devices interpret your reply as valid HTTP, not random text.


## 3. get_query_param (Extract query string values like ?state=on)

When a client sends GET /led?state=on, we must pull out state=on.

Otherwise, the server wouldn’t know the client’s intention (turn LED on vs off).

📌 Why needed? To interact dynamically with clients — sensors, toggles, etc. If we can’t parse queries, our server is just a “file dumper.”


## 4.parse_http_request (Split raw request into method, path, query)

Raw request from client looks like this:

GET /temperature HTTP/1.1
Host: 192.168.0.10


We must parse out:

Method (GET)

Path (/temperature)

Query params (?state=on)

📌 Why needed? This is how the server decides what action to take (send sensor value, toggle LED, return error).


## 5.handle_client (Main workflow per request)

This is the heart of the server loop:

Check rate limit → prevent abuse/spam.(optional)

Receive request → read raw HTTP text from socket.

Parse request → understand what client wants.

Route to handler → if /led, toggle LED; if /temperature, read sensor.

Respond → send back HTTP headers + JSON/HTML.

Cleanup → close socket to free resources.

📌 Why needed? This creates the flow of interaction between client and server. Without this orchestration, your server either crashes or doesn’t know how to respond.

