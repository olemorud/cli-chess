
#include <locale.h>
#include <string.h>
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


void print_board(int* board){
	wchar_t piece;

	// The unicode value for a white chess king is 0x2654
	// the following unicode 5 unicode characters are the 
	// other white chess pieces. (matches macro definitions)
	// (https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode)

	for(int i=0; i<8; i++){
		printf(" %i ", 8-i); // number coordinates

		for(int j=0; j<8; j++){	
			int p = board[i*8+j];
			
			// set tile color
			if((i+j) % 2)
				printf("\033[100;39m"); // "light black"
			else
				printf("\033[107;39m"); // "light white"

			if(p == E){
				putchar(' ');
			}else{
				if(p > 0){
					printf("\033[37m"); // white foreground
				}else{
					printf("\033[30m"); // black foreground
					p *= -1;
				}

				piece = 0x2659 + p; // unicode for black king is 0x267a (0x2659 + 1).
									// The macro definitions of pieces align with 
									// unicode values to display the correct piece.
				printf("%lc", piece);
			}

			putchar(' ');
		}

		printf("\033[0m"); // reset text attributes
		putchar('\n');
	}

	printf("\033[0m"); // reset text attributes
	
	// print letter coordinates
	printf("  ");
	for(int i=0; i<8; i++) printf(" %c", 'A'+i);
}



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


int main(){
	setlocale(LC_ALL, "C.UTF-8");

	int *board;
	init_board(board);
	print_board(board);

	return 0;
}
