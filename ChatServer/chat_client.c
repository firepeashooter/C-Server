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
#include <pthread.h> //for multi threading oooo

#define PORT "3490" //Make sure that this lines up with what the server is

// Global variables so the background thread can access the UI
WINDOW *chat_win;
WINDOW *input_win; //
int num_messages = 2; // Make sure this is global now so both loops can update it
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;

void* listen_server(void* arg) {
    int sockfd = *(int*)arg;
    char recv_buffer[512];
    
    while (1) {
        // This blocks until the server sends a message
        int bytes_received = recv(sockfd, recv_buffer, sizeof(recv_buffer) - 1, 0);
        
        if (bytes_received > 0) {
            recv_buffer[bytes_received] = '\0'; // Safety null-terminator
												//
			pthread_mutex_lock(&screen_mutex);
            
            int chat_y, chat_x;
            getmaxyx(chat_win, chat_y, chat_x);

            // 1. Handle scrolling exactly like we did before
            if (num_messages >= chat_y - 2) {
                wscrl(chat_win, 1);
                num_messages = chat_y - 3;
            }

            // 2. Print the message from the server directly into the chat window
            mvwprintw(chat_win, num_messages, 4, "%s", recv_buffer);
            
            // 3. Redraw the box over the scrolled area
            box(chat_win, 0, 0);
            mvwprintw(chat_win, 1, 1, "Chat History"); 
            wrefresh(chat_win);
			wrefresh(input_win);

            num_messages++;

			pthread_mutex_unlock(&screen_mutex);
            
        } else if (bytes_received == 0) {
            // Server disconnected gracefully
            break;
        } else {
            // Socket error
            break;
        }
    }
    return NULL;
}

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

    chat_win = newwin(y - 2, x, 0, 0);  // Changed from y - 3 to y - 2
    input_win = newwin(3, x, y - 3, 0);

    scrollok(chat_win, TRUE);

    // 3. IMPORTANT: Refresh the background first
    refresh();

    // 4. Draw to Chat Window
    box(chat_win, 0, 0);
    mvwprintw(chat_win, 1, 1, "Chat History");
    wrefresh(chat_win); // Show the chat window

    // 5. Draw to Input Window
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "Input: ");
	wmove(input_win, 1, 8);
    wrefresh(input_win); // Show the input window
						 //
	int num_messages = 2; //Starts at the offset we need for the input window
	int max_y, max_x;
	getmaxyx(chat_win, max_y, max_x);
	
	char msg_buffer[256];
	int msg_indx = 0;


	pthread_t thread_id;
	pthread_create(&thread_id, NULL, listen_server, (void*)&sockfd);


    // 6. Wait loop that waits for user input
	while(1){

		int ch = wgetch(input_win);
		
		if (ch == KEY_F(1)) break;

		if (ch == '\n'){

			msg_buffer[msg_indx] = '\0';
        
			// 1. Send the message to the server
			send_message(sockfd, msg_buffer);

    	    pthread_mutex_lock(&screen_mutex);
			// 2. Clear the input box and get ready for the next message
			wclear(input_win);
			msg_indx = 0;

			box(input_win, 0, 0);
			mvwprintw(input_win, 1, 1, "Input: ");
			wmove(input_win, 1, 8);
			wrefresh(input_win);		

			pthread_mutex_unlock(&screen_mutex);

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
