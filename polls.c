#include <asm-generic/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>


#define PORT "3490"




int add_client(int listener){


	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int new_fd;


	new_fd = accept(listener, (struct sockaddr *)&their_addr, &addr_size);

	if ( new_fd == -1){
		perror("accept");
		printf("Failed to connect User");
	}

	//Add the new socket to the pfds array
	






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
		if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
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


int process_connections(int listener, int num_events, int pfds_count, struct pollfd* pfds){


	for(int i = 0; i < pfds_count; i++){
		
		//Update found!
		if (pfds[i].revents & POLLIN){


			//If it's the listener, accept and then add it to the array
			if (pfds[i].fd == listener){

				

			//Broadcast to every one in the array except for myself
			}else{

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

	int num_events = poll(pfds, pfds_count, 2500);

	process_connections(listener, num_events, pfds_count, pfds);






	//Tells the struct at position 0 to watch the 0 file descriptor (standard in)
	pfds[0].fd = 0; 
	pfds[0].events = POLLIN;

	printf("Hit RETURN or wait 2.5 seconds for timeout\n");
	
	int num_events = poll(pfds, 1, 2500); // 2.5 second timeout
	
	if (num_events == 0) {
		printf("Poll timed out!\n");
	} else {
		int pollin_happened = pfds[0].revents & POLLIN;
	
		if (pollin_happened) {
			printf("File descriptor %d is ready to read\n", pfds[0].fd);
		} else {
			printf("Unexpected event occurred: %d\n", pfds[0].revents);
		}
	}
	
	return 0;




}
