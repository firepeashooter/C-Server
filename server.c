#include <stdio.h>      // For printf/errors
#include <string.h>     // For memset
#include <sys/types.h>  // For socket types
#include <sys/socket.h> // For socket(), connect()
#include <netdb.h>      // For getaddrinfo() and struct addrinfo
#include <unistd.h>     // For close()




int main(){

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


	if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
		perror("server: bind");
		close(sockfd);
		freeaddrinfo(res);
		return 3;
	}


	freeaddrinfo(res);

	if (listen(sockfd, 10) == -1){
		perror("server: listen");
		close(sockfd);
		return 4;
	}

	printf("Server: waiting for conections...\n");



	//HANDLING THE CONNECTION
	while(1){

		addr_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

		if (new_fd == -1){
			perror("accept");
			continue;
		}

		printf("Server: got connection from: '%d'\n", new_fd);

		//Recieving Data
		numbytes = recv(new_fd, buf, 1023, 0);

		if (numbytes == -1){
			perror("recv");
			close(new_fd);
			continue;
		};


		buf[numbytes] = '\0';

		printf("Server: received '%s'\n", buf);

		close(new_fd);


		




	}


}


