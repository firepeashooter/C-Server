#include <stdio.h>      // For printf/errors
#include <string.h>     // For memset
#include <sys/types.h>  // For socket types
#include <sys/socket.h> // For socket(), connect()
#include <netdb.h>      // For getaddrinfo() and struct addrinfo
#include <unistd.h>     // For close()
#include <stdlib.h>
#include <poll.h>



#define PORT "3490" //Make sure that this lines up with what the server is

int get_socket(){

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

	//this is the dns lookup
	if((status = getaddrinfo("bens-chat-server.duckdns.org", PORT, &hints, &res)) != 0){
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

	freeaddrinfo(res);

	return sockfd;


};

int main(int argc, char* argv[]){

	if (argc != 2){
		fprintf(stderr, "Usage: %s <username>\n", argv[0]);
		return 1;
	}
	
	//grab a socket
	int sockfd = get_socket();


	//first they send their username
	char* username = argv[1];

	int len;
	len = strlen(username);

	int total_sent = 0;
	int bytes_left = len;
	int n;

	while(total_sent < len) {
		n = send(sockfd, username + total_sent, bytes_left, 0);
		if (n == -1) { break; } // Handle error
		total_sent += n;
		bytes_left -= n;
	}


	//we want to poll the standard input and our socket
	
	//sets up the array of pfds
	struct pollfd pfds[2];

	pfds[0].fd = 0;
	pfds[0].events = POLLIN;

	pfds[1].fd = sockfd;
	pfds[1].events = POLLIN;

	char msg_buffer[1024];

	//Or just loop while the server is open we need to disconnect when this happens
	while (1) {


		int num_events = poll(pfds, 2, 2500);


		if (pfds[0].revents & POLLIN){ //If the standard input is happening
									   //
			puts("Input Detected");
			if (fgets(msg_buffer, sizeof(msg_buffer), stdin) == NULL) break;

			//How the user exits
			if (strcmp(msg_buffer, "exit\n") == 0) break;

			int msg_len = strlen(msg_buffer);
			int total_msg_sent = 0;

			// Send the message directly from the buffer
			while (total_msg_sent < msg_len) {
				int n = send(sockfd, msg_buffer + total_msg_sent, msg_len - total_msg_sent, 0);
				if (n <= 0) {
					perror("Server disconnected");
				}
				total_msg_sent += n;
			}


		}

		if (pfds[1].revents & POLLIN){ //Else it's the server

			puts("Server Detected");
			//display the server message

			char buf[1023];

			int nbytes = recv(pfds[1].fd, buf, sizeof(buf) - 1, 0);

			buf[nbytes] = '\0';

			printf("Message Received: %s", buf);
		}



		// printf("Message: ");
		// fflush(stdout);

			}


}









