#include <asm-generic/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>


#define PORT "3490"




int add_client(int listener, int* pfds_count, struct pollfd* pfds){


	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int new_fd;


	new_fd = accept(listener, (struct sockaddr *)&their_addr, &addr_size);

	if ( new_fd == -1){
		perror("accept");
		printf("Failed to connect User");
	}

	//Add the new socket to the pfds array
	//TODO: MAKE IT SO THAT IT ERRORS OUT OF BOUNDS
	pfds[*pfds_count].fd = new_fd;
	pfds[*pfds_count].events = POLLIN;


	printf("New connection added to the Group using socket %d", new_fd);

	return 0;
}

void process_client_data(int listener, int *pfds_count, struct pollfd* pfds, int *pfd_i){

	//buffer for client data
	char buf[256];

	//Recieve the message
	int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);
	//Record who sent it (so we don't send the message back to them)
	int sender_fd = pfds[*pfd_i].fd;

	if (nbytes <= 0){ //Any error or connection closed
		if (nbytes == 0){
			printf("Server: socket %d hung up\n", sender_fd);
		}else{
			perror("recv");
		}

		close(pfds[*pfd_i].fd); //Close this socket
	
		//TODO: Delete this socket from our list

	} else{ //Good Client data
		
		printf("server: recv from fd %d: %.*s", sender_fd, nbytes, buf);

		for (int j = 0; j < *pfds_count; j++){
			int dest_fd = pfds[j].fd;

			//Except the listener and outselves
			if (dest_fd != listener && dest_fd != sender_fd){
				if (send(dest_fd, buf, nbytes, 0) == -1){
					perror("send");
				}
			}
		}

	}
}


int make_listener_socket(){

	//Filling in values for the structs
	struct addrinfo hints, *res, *p;
	int sockfd;
	int status;
	int listener;
	int yes = 1;
	socklen_t addr_size;
	struct sockaddr_storage their_addr;
	char buf[1024];



	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0){

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}


	//Loop through all the results and bind to the first one that we can
	for (p = res; p != NULL; p = p->ai_next){

		//Try to get a socket
		listener = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if (listener == -1){
			continue;
		}

		//Losing the address already in use error
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		//Try to bind
		if ((bind(listener, res->ai_addr, res->ai_addrlen)) == -1){
			close(listener);
			continue;
		}
	}

	if (p == NULL){
		return -1;
	}

	freeaddrinfo(res);

	if (listen(listener, 10) == -1){
		return -1;
	}

	return listener;

}


int process_connections(int listener, int num_events, int* pfds_count, struct pollfd* pfds){


	for(int i = 0; i < *pfds_count; i++){
		
		//Update found!
		if (pfds[i].revents & POLLIN){


			//If it's the listener, accept and then add it to the array
			if (pfds[i].fd == listener){

				add_client(listener, pfds_count, pfds);

				

			//Broadcast to every one in the array except for myself
			}else{

				process_client_data(listener, pfds_count, pfds, &i);


			};
		}




	}


}

int main(void){


	//Creates an array of pollfd structs
	struct pollfd pfds[5];
	int pfds_count = 0;


	int listener = make_listener_socket();

	pfds[0].fd = listener;
	pfds[0].events = POLLIN;

	pfds_count += 1;

	puts("Server is Waiting for Connections....");

	for (;;){

		int num_events = poll(pfds, pfds_count, 2500);
		process_connections(listener, num_events, &pfds_count, pfds);
	}


}




