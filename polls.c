#include <stdio.h>
#include <poll.h>





int main(void){


	//Creates an array of pollfd structs
	struct pollfd pfds[1];

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
