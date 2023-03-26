
#include "threatmap.h"

// uint64_t pawn_threatmap(const tile_t board[BOARD_SIZE], index_t index);
// uint64_t bishop_threatmap(const tile_t board[BOARD_SIZE], index_t index);
// uint64_t rook_threatmap(const tile_t board[BOARD_SIZE], index_t index);
// uint64_t knight_threatmap(index_t index);
// uint64_t king_threatmap(const tile_t board[BOARD_SIZE], index_t index);
// uint64_t queen_threatmap(const tile_t board[BOARD_SIZE], index_t index);
// uint64_t threatmap(const tile_t board[BOARD_SIZE], int color_attacking);
// void print_threatmap(uint64_t threatmap);

void test_threatmap()
{
    const tile_t e4[] = {
           -R, -N, -B, -Q, -K, -B, -N, -R,
           -P, -P, -P, -P, -P, -P, -P, -P,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  P,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            P,  P,  P,  E,  P,  P,  P,  P,
            R,  N,  B,  Q,  K,  B,  N,  R 
    };

    const tile_t lonely_bishop[] = {
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  B,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
    };

    const tile_t lonely_rook[] = {
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  R,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,
    };

    (void)lonely_rook; (void)lonely_bishop; (void)e4;

	//print_threatmap(threatmap(e4, WHITE));
	print_threatmap(threatmap(lonely_bishop, WHITE));
	//print_threatmap(threatmap(lonely_rook, WHITE));
}

int main()
{
	test_threatmap();

	return 0;
}
