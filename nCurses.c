#include <stdio.h>
#include <ncurses.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>     // For memset
#include <sys/poll.h>
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
	if((status = getaddrinfo("192.168.2.10", PORT, &hints, &res)) != 0){
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

int send_message(int sockfd, char* message){

	int msg_len = strlen(message);

	int total_sent = 0;
	int bytes_left = msg_len;
	int n;

	while (total_sent < msg_len){
		n = send(sockfd, message + total_sent, bytes_left, 0);
		if (n == -1) {break;}
		total_sent += n;
		bytes_left -= n;
	}

	n = send(sockfd, "\n", 1, 0);
    if (n == -1) { return -1; }

	return 0;

}

int main(int argc, char* argv[]) {

	//Error handles the username
	if (argc != 2){
		fprintf(stderr, "Usage: %s <username>\n", argv[0]);
		return 1;
	}

	char* username = argv[1];

	puts(username);
	int sockfd = get_socket();

	//Send the username
	send_message(sockfd, username);

    // 1. Setup
    initscr();
    cbreak();
    noecho();

	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/

	//For holding input
	int ch;

    int y, x;
    getmaxyx(stdscr, y, x);

    // 2. Create Windows
    WINDOW *chat_win = newwin(y - 3, x, 0, 0);
    WINDOW *input_win = newwin(3, x, y - 3, 0);

    // 3. IMPORTANT: Refresh the background first
    refresh();

    // 4. Draw to Chat Window
    box(chat_win, 0, 0);
    mvwprintw(chat_win, 1, 1, "Chat History");
    wrefresh(chat_win); // Show the chat window

    // 5. Draw to Input Window
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "Input: ");
    wrefresh(input_win); // Show the input window
						 //
	int num_messages = 2; //Starts at the offset we need for the input window
	int max_y, max_x;
	getmaxyx(chat_win, max_y, max_x);
	
	char msg_buffer[256];
	int msg_indx = 0;


    // 6. Wait loop that waits for user input
	while(1){

		int ch = wgetch(input_win);
		
		if (ch == KEY_F(1)) break;

		if (ch == '\n'){

			if (num_messages > 10){
				num_messages = 10;
			}	

			msg_buffer[msg_indx] = '\0';
			//send the message to the server
			send_message(sockfd, msg_buffer	);

			mvwprintw(chat_win, max_y - num_messages, 2, "You: %s", msg_buffer);
			wrefresh(chat_win);
			wclear(input_win);

			//Flush the buffer
			msg_indx = 0;

			num_messages++;

			//Redraw the input
			box(input_win, 0, 0);
			mvwprintw(input_win, 1, 1, "Input: ");
			wmove(input_win, 1, 8);  // 3. Put the cursor back in the "home" position
			wrefresh(input_win); 

		//handle backspace
		}else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b'){

			if (msg_indx > 0){
				msg_indx--;                  // Move index back
				msg_buffer[msg_indx] = '\0'; // Your clean memory reset

				// 1. Move the window's internal cursor back one space
				int cur_y, cur_x;
				getyx(input_win, cur_y, cur_x);
				wmove(input_win, cur_y, cur_x - 1);

				// 2. Wipe the character under the cursor and pull everything back
				wdelch(input_win);

				// 3. Render the change to the monitor
				wrefresh(input_win);
			}


		} else{
			
			//Add to the buffer
			if (msg_indx < 256){
				msg_buffer[msg_indx++] = ch;
			}

			waddch(input_win, ch);
			wrefresh(input_win);

		}

	}

    // 7. Exit
    endwin();
    return 0;
}
