
#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>


#define E 0 /* empty */
#define K 1 /* king */
#define Q 2 /* queen  */
#define R 3 /* rook */
#define B 4 /* bishop */
#define N 5 /* knight */
#define P 6 /* pawn */

#define WHITE 1
#define BLACK -1

int get_piece(char* str);
void setcolor(int mode, int r, int g, int b);
void print_board(int* board);
void init_board(int* board);
void do_turn(int turn_no, int* board);


/*
 * main
 */
int main(){
	int turn = 0;

	setlocale(LC_ALL, "C.UTF-8");

	int *board;
	init_board(board);

	while(1){
		print_board(board);
		do_turn(turn++, board);
	}

	return 0;
}


/*
 * Sets the foreground or background color.
 * Modes:
 *  - 0: change background
 *  - 1: change foreground
 *  - 2: reset colors
 */
void setcolor(int mode, int r, int g, int b){
	if( mode == 2 )
		printf("\033[0m");
	else
		printf("\033[%i;2;%i;%i;%im", mode?38:48, r, g, b);
};


/*
 * Prints the board
 */
void print_board(int* board){
	/*
		The loop checks if the tile is empty and prints ' ' if so.
		Otherwise the foreground color is set to match the player
		and the unicode symbol for the piece is printed.

		The unicode value for a fullcolor chess king is 0x2654
		the following unicode 5 unicode characters are the 
		other white chess pieces. (matches macro definitions)
		(https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode) 

		The unicode symbol is calculated from adding 0x2653 with the
		piece value.
	*/
	for(int i=0; i<8; i++){
		printf("\n %i ", 8-i); // print number coordinates on y-axis

		for(int j=0; j<8; j++){	
			int p = board[i*8+j];
			
			// Make tiles black and white 
			if((i+j) % 2)
				setcolor(0, 100, 100, 150);
			else
				setcolor(0, 150, 150, 200);


			// Print empty space and do nothing if tile is empty
			if(p == E){
				printf("  ");
				continue;
			}

			// Set piece color
			if(p > 0){
				setcolor(1, 0xff, 0xff, 0xff); //white
			}else{
				setcolor(1, 0, 0, 0); //black
				p *= -1;
			}
			
			printf("%lc ", 0x2659 + p);
		}

		setcolor(2, 0, 0, 0); // reset text attributes
	}
	
	// Print horizontal letter coordinates
	printf("\n  ");
	for(int i=0; i<8; i++) printf(" %c", 'a'+i);
}


/*
 * Resets/inits the board
 */
void init_board(int *board){

	// black pieces are prefixed by a minus (-)
	const int start[] = {
		-R,-N,-B,-Q,-K,-B,-N,-R, 
		-P,-P,-P,-P,-P,-P,-P,-P, 
		 E, E, E, E, E, E, E, E, 
		 E, E, E, E, E, E, E, E, 
		 E, E, E, E, E, E, E, E, 
		 E, E, E, E, E, E, E, E, 
		 P, P, P, P, P, P, P, P, 
		 R, N, B, Q, K, B, N, R
	};

	memcpy(board, start, sizeof(start));
}


// TODO: Implement algebaric notation	
/*
 * Get move, check move and log move for turn <turn_no>
 */
void do_turn(int turn_no, int* board){
	char input[3] = { 0 };
	int from = -1, to = -1, tmp;

	printf("\nPlayer %i, your turn to move", 1 + turn_no%2);
	
	/* temporary and ugly solution - read from and to */
	while(from == -1 || to == -1){
		from = to = -1;

		printf("\nMove piece\nfrom: ");
		scanf(" %2s", input);
		tmp = get_piece(input);
		if(tmp == -1){
			printf("bad value");
			continue;
		}
		from = tmp;
		
		printf("\nto: ");
		scanf(" %2s", input);
		tmp = get_piece(input);
		if(tmp == -1){
			printf("bad value");
			continue;
		}
		to = tmp;
	}

	board[to]   =  board[from];
	board[from] = E;

}


/*
 * Translates A1, 3B etc. to the 1D index of the board
 */
int get_piece(char *str){
	int x = -1,
		y = -1,
		c;

	for(int i=0; i<2; i++){
		c = str[i];

		if(isalpha(c)) c = toupper(str[0]);

		if( 'A' <= c && c <= 'H' ){
			x = c - 'A';
		}else if('1' <= c && c <= '8'){
			y = c - '1';
		}
	}

	if(x != -1 && y != -1){
		return 8*(7-y) + x;
	}else{
		return -1;
	}
}
