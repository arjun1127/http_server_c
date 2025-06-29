#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unisted.h>
#include <stdlib.h>
#include <string.h>

#define LISTENADDR "127.0.0.1"
/* returns 0 on error else it returns a socket fd */

int srv_init(int portno){
 int s;
 struct sockaddr_in srv;

 s=socket(AF_INET,SOCK_STREAM,0);
 if(s<0)
  return 0;

}

int main(int argc,char *argv[]){

 int s;
 char *port;
 if(argc<2){
  fprintf(stderr,"Usage: %s <listening port>\n",argv[0]);
  return -1;
 }else 
   port = argv[1];
 s=srv_init(atoi(port));

}
