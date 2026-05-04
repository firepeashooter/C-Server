#include <stdio.h>
#include <ncurses.h>
#include <ncurses.h>
#include <unistd.h>

int main() {
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



    // 6. Wait
	while(1){

		int ch = wgetch(input_win);
		
		if (ch == KEY_F(1)) break;

		if (ch == '\n'){

			if (num_messages > 10){
				num_messages = 10;
			}	

			msg_buffer[msg_indx] = '\0';
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
