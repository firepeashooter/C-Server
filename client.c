#include <stdio.h>      // For printf/errors
#include <string.h>     // For memset
#include <sys/types.h>  // For socket types
#include <sys/socket.h> // For socket(), connect()
#include <netdb.h>      // For getaddrinfo() and struct addrinfo
#include <unistd.h>     // For close()




int main(int argc, char* argv[]){

	//SETTING UP THE SOCKET
	
	//fills the structs we need for what kind of connection we want
	struct addrinfo hints, *res;
	int sockfd;
	int status;
	int new_fd;
	socklen_t addr_size;
	struct sockaddr_storage their_addr;
	char buf[1024];
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//this is the dns lookup
	if((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0){

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	};

	//Make a socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (sockfd == -1){
		perror("server: socket");
		freeaddrinfo(res);
		return 2;
	};

	if ( (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)){
		perror("server: connect");
		freeaddrinfo(res);
		return 3;
	}


	char* msg = argv[1];
	int len, bytes_sent;

	len = strlen(msg);

	bytes_sent = send(sockfd, msg, len, 0);

}
