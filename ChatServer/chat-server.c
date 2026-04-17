#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>


#define PORT "3490" //TODO CHANGE THIS TO 6969?

struct client{
	int fd;
	char username[100];
};


int send_message(int sockfd, char* msg){

	int msg_len = strlen(msg);
	int total_msg_sent = 0;

	// Send the message directly from the buffer
	while (total_msg_sent < msg_len) {
		int n = send(sockfd, msg + total_msg_sent, msg_len - total_msg_sent, 0);

		if (n <= 0) {
			perror("Server disconnected");
			return -1;
		}

		total_msg_sent += n;
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


	//Setting hints and clearing memory
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0){

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}


	//Loop through all the results and bind to the first one that we can
	for (p = res; p != NULL; p = p->ai_next){

		//Try to get a socket
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (listener == -1){
			continue;
		}

		//Losing the address already in use error
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		//Try to bind
		if ((bind(listener, p->ai_addr, p->ai_addrlen)) == -1){
			close(listener);
			continue;
		}

		//Once bound we cna exit the loop	
		break;
	}

	if (p == NULL){
		return -1;
	}

	freeaddrinfo(res);

	//Start the socket listening
	if (listen(listener, 10) == -1){
		return -1;
	}

	return listener;
}

int add_client(int* pfds_count, int* pfds_total_count, struct pollfd* pfds, int socket, struct client* clients, char* username){

	//Add socket to pfds array
	

	//And do the same with the client
	
	if (pfds_count == pfds_total_count){ //If we go beyond the bounds of the array realloc double
		
		pfds = realloc(pfds, (*pfds_total_count * 2) * sizeof(struct pollfd));
		clients = realloc(clients, (*pfds_total_count * 2) * sizeof(struct client));

		if (pfds == NULL){
			printf("Failed to reallocate memory for the polls array");
			exit(-1);
		}

		if (clients == NULL){
			printf("Failed to reallocate memory for the client array");
			exit(-1);
		}

		pfds[*pfds_count].fd = socket;
		pfds[*pfds_count].events = POLLIN;

		clients[*pfds_count].fd = socket;
		//Setting username or something
		strncpy(clients[*pfds_count].username, username, sizeof(clients[*pfds_count].username) - 1);
		clients[*pfds_count].username[sizeof(clients[*pfds_count].username) - 1] = '\0';

		(*pfds_count)++;


		return 0;

	}else{ //Just add the item to the array

		pfds[*pfds_count].fd = socket;
		pfds[*pfds_count].events = POLLIN;

		clients[*pfds_count].fd = socket;
		//Setting username or something
		strncpy(clients[*pfds_count].username, username, sizeof(clients[*pfds_count].username) - 1);
		clients[*pfds_count].username[sizeof(clients[*pfds_count].username) - 1] = '\0';

		(*pfds_count)++;

		return 0;
	}
}

int remove_client(int socket, struct pollfd* pfds, int* pfds_count, struct client* clients){

	//Notify the clients that we are removing the current person
	char goodbye_message[500];
	char* username;
	
	//Loop through the array and if the socket matches then we remove that member from the list and decremenet pfds_count
	for(int i = 0; i < *pfds_count; i++){

		if (pfds[i].fd == socket){

			//grab the username
			username = clients[i].username;
			
			//Format the message
			snprintf(goodbye_message, sizeof(goodbye_message), "%s Left the Chat\n", username);

			//Send clients except themselves that they left
			for (int j = 0; j < *pfds_count; j++){

				if (j ==0){ //if we are the listener
					continue;
				} 

				if (pfds[j].fd == socket){
					continue;
				} else {
					send_message(pfds[j].fd, goodbye_message);
				}
			}
			
			//Remove the user
			pfds[i] = pfds[*pfds_count -1];
			clients[i] = clients[*pfds_count -1];
			(*pfds_count)--;


			return 0;

		}

	}

	return -1;

}



int process_client(int listener, int* pfds_count, int* pfds_total_count, struct pollfd* pfds, struct client* clients){

	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	int new_fd;

	//Accept accepts the connection, and writes the user connection to the their_addr struct
	new_fd = accept(listener, (struct sockaddr *)&their_addr, &addr_size);

	if (new_fd == -1){
		perror("accept");
		printf("Failed to connect User");
		fflush(stderr);
		return -1;
	}

	
	
	//recv their username
	char username[1023];
	char welcome_message[500];
	char self_welcome_message[500];

	

	//TODO: We should probably loop this to get the whole thing if it's long
	
	//Recieves the username and add's a null character
	int nbytes = recv(new_fd, username, sizeof(username), 0);
	if (nbytes > 0) {
		username[nbytes] = '\0'; 
	}

	//Format the strings
	snprintf(welcome_message, sizeof(welcome_message), "%s Joined the Chat\n", username);
	snprintf(self_welcome_message, sizeof(welcome_message), "Joined the Chat as %s\n", username);

	printf("New connection added to the Group using socket %d: USERNAME: %s ", new_fd, username);
	fflush(stdout);

	//Add the new socket to the pfds array
	if (add_client(pfds_count, pfds_total_count, pfds, new_fd, clients, username) == -1){
		fprintf(stderr, "Failed to add client");
		exit(1);
	}

	//Send clients except themselves that someone joined
	for (int i = 0; i < *pfds_count; i++){

		if (pfds[i].fd == listener){ //if we are the listener
			continue;
		} 

		if (pfds[i].fd == new_fd){
			send_message(new_fd, self_welcome_message);
		} else {
			send_message(pfds[i].fd, welcome_message);
		}
	}
	return 0;

}

void process_client_data(int listener, int *pfds_count, struct pollfd* pfds, int *pfd_i, struct client* clients){

	//buffer for client data
	char buf[256];

	//Recieve the message
	int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);
	//Record who sent it (so we don't send the message back to them)
	int sender_fd = pfds[*pfd_i].fd;
	char* username = clients[*pfd_i].username;

	if (nbytes <= 0){ //Any error or connection closed
		if (nbytes == 0){
			printf("Server: socket %d hung up\n", sender_fd);
		}else{
			perror("recv");
		}

		int fd_to_close = pfds[*pfd_i].fd;
	
		remove_client(fd_to_close, pfds, pfds_count, clients);

		close(fd_to_close); //Close this socket

	} else{ //Good Client data
		
		printf("server: recv from fd %d: Username: %s | Message: %.*s", sender_fd, username, nbytes, buf);

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


void process_connections(int listener, int* pfds_count, int* pfds_total_count, struct pollfd* pfds, struct client* clients){


	for(int i = 0; i < *pfds_count; i++){
		//Update found!
		if (pfds[i].revents & POLLIN){

			//If it's the listener, add it to the array
			if (pfds[i].fd == listener){

				process_client(listener, pfds_count, pfds_total_count, pfds, clients);

			//Broadcast to every one in the array except for myself
			}else{

				process_client_data(listener, pfds_count, pfds, &i, clients);

			};
		}
	}
}

int main(void){

	//Creates an dynamic array of pollfd structs
	int pfds_total_size = 5;
	struct pollfd* pfds = (struct pollfd*) malloc(pfds_total_size * sizeof(struct pollfd));

	//And an array of clients
	int clients_total_size = 5;
	struct client* clients = (struct client*) malloc(clients_total_size * sizeof(struct client)); 	

	if (pfds == NULL){
		printf("Memory for polling array failed to allocate.\n");
		exit(1);
	}

	if (clients == NULL){
		printf("Memory for client array failed to allocate.\n");
		exit(1);
	}

	int pfds_count = 0;
	int clients_count = 0;


	int listener = make_listener_socket();

	pfds[0].fd = listener;
	pfds[0].events = POLLIN;

	pfds_count += 1;

	puts("Server is Waiting for Connections....");

	for (;;){
		int num_events = poll(pfds, pfds_count, 2500); //Server waits on this line till a port has activity
		process_connections(listener, &pfds_count, &pfds_total_size, pfds, clients);
	}


}




