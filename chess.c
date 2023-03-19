
#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef int32_t tile_t;
typedef ssize_t pos_t;

#define ROW ( (pos_t) 8 )
#define COL ( (pos_t) 8 )

#define BOARD_SIZE ROW * COL * sizeof(tile_t)

#define E ( (tile_t) 0 ) /* empty */
#define K ( (tile_t) 1 ) /* king */
#define Q ( (tile_t) 2 ) /* queen  */
#define R ( (tile_t) 3 ) /* rook */
#define B ( (tile_t) 4 ) /* bishop */
#define N ( (tile_t) 5 ) /* knight */
#define P ( (tile_t) 6 ) /* pawn */

#define WHITE 1
#define BLACK -1

#define UNICODE_CHESS_SYMBOL 0x2659


int  get_piece(char* input);

void setcolor(int mode, int r, int g, int b);

void print_board(tile_t* board);

void init_board(tile_t* board);

void do_turn(int turn_no, tile_t *board);

int  move_ok(tile_t* board, pos_t from, pos_t to, int player);

bool tile_empty(tile_t t);

int  column(pos_t t);

int  row(pos_t t);

bool pawn_move_ok(tile_t const *board, pos_t from, pos_t to, int direction);

/*
 * main
 */
int main(){
	int turn = 0;

	setlocale(LC_ALL, "C.UTF-8");

	tile_t *board = malloc(BOARD_SIZE);

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
void setcolor(const int mode, const int r, const int g, const int b){
	if( mode == 2 )
		printf("\033[0m");
	else
		printf("\033[%i;2;%i;%i;%im", mode?38:48, r, g, b);
};


/*
 * Prints the board
 */
void print_board(tile_t* board){
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
	for(size_t i=0; i<8; i++){
		printf("\n %zu ", 8-i); // print number coordinates on y-axis

		for(size_t j=0; j<8; j++){
			tile_t p = board[i*8+j];
			
			// Make tile black and white
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
			
			printf("%lc ", UNICODE_CHESS_SYMBOL + p);
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
void init_board(tile_t *board){

	// black pieces are prefixed by a minus (-)
	const tile_t start[] = {
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
void do_turn(int turn_no, tile_t *board){
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

		if(!move_ok(board, from, to, turn_no%2?BLACK:WHITE )){
			printf("bad value");
			from = -1;
			continue;
		}
	}

	board[to]   =  board[from];
	board[from] = E;

}


/*
 * Translates A1, 3B etc. to the 1D index of the board
 */
int get_piece(char *input){
	int x = -1,
		y = -1,
		c;

	for(int i=0; i<2; i++){
		c = input[i];

		if(isalpha(c)) c = toupper(input[0]);

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


/*
 * Returns 1 if a move is valid, 0 otherwise
 */
int move_ok(tile_t* board, pos_t from, pos_t to, int const player)
{

	printf("attempting to move %i to %i", board[from], board[to]);

	switch (board[from]) {
	default:
		return 0;
		break;

	// PAWNS
	case P: case -P:
		return pawn_move_ok(board, from, to, player);

	// the remaining pieces are left as an exercise for the reader 
	}

	return 0;
}

/* Returns true if tile is empty */
bool tile_empty(tile_t t)
{
	return t == E;
}

/* Returns row number of board index */
int row(pos_t t)
{
	return t/ROW;
}

/* Returns column number of board index */
int column(pos_t t)
{
	return t/COL;
}

/* Returns true if a and b are tiles of opposite player */
bool are_opponents(tile_t a, tile_t b)
{
	return a*b < 0;
}

/* Returns true if move is a valid pawn move
    board     - array of tiles representing chess board state
    from      - index of board piece starts at
    to        - index of board piece wants to move to
    direction - pawns movement direction
 *  */
bool pawn_move_ok(tile_t const* board, pos_t from, pos_t to, int direction)
{
	pos_t const diff = (from - to) * direction;

	switch (diff) {
	default:
		return false;

	case ROW: /* single move */
		return tile_empty(board[to]);

	case ROW - 1: case ROW+1: /* diagonal attack */
		return are_opponents(board[to], board[from]);

	case 2 * ROW: /* double move */
		return tile_empty(board[to])
		    && tile_empty(board[from - ROW * direction])
		    && (row(from) == 1 || row(from) == 6 );
	}
}

/* WIP */
bool bishop_move_ok(tile_t const* board, pos_t from, pos_t to)
{
	pos_t diff = (from - to);
	
	/* diagonal moves change row and col equally */
	if (row(diff) != column(diff))
		return false;

	return false;
}
