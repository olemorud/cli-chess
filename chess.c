
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#define E 0 /* empty  */
#define K 1 /* king */
#define Q 2 /* queen  */
#define R 3 /* rook */
#define B 4 /* bishop */
#define N 5 /* knight */
#define P 6 /* pawn */

#define WHITE 1
#define BLACK -1

int main(){
	int *board;
	init_board(board);
	print_board(board);

	return 0;
}



void print_board(int* board){
	wchar_t piece;
	
	// The unicode value for a white chess king is 0x2654
	// the following unicode 5 unicode characters are the 
	// other white chess pieces. (matches macro definitions)
	// (https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode)
	for(int i=0; i<8; i++){
		for(int j=0; j<8; j++){
			piece = 0x2654 + board[i];
			putwchar(piece);
		}
		putchar('\n');
	}
}



void init_board(int *board){
	
	// black pieces are prefixed by minus (-)
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
