
#include <ctype.h>   /* isalpha, isdigit ... */
#include <locale.h>  /* setlocale */
#include <stdio.h>   /* printf, scanf */
#include <string.h>  /* memcpy */

#include "common.h"
#include "graphics.h"
#include "pieces.h"

void    do_turn(int turn_no, tile_t board[BOARD_SIZE]);
void    init_board(tile_t board[BOARD_SIZE]);
index_t input_to_index(char input[2]);

int main()
{
    int turn = 0;

    setlocale(LC_ALL, "C.UTF-8");

    tile_t board[8 * 8] = { 0 };

    init_board(board);

    while (true) {
        printf("\033[2J"); // clear screen
        print_board(board);
        do_turn(turn, board);
        ++turn;
    }

    return 0;
}

/**
 * Resets/initializes the board
 *
 * Sets all the tiles of the board to match the starting position of chess.
 *
 * \param board Pointer to list of tiles representing board state
 */
void init_board(tile_t board[BOARD_SIZE])
{
    // black pieces are prefixed by a minus (-)
    const tile_t start[]
        = { -R, -N, -B, -Q, -K, -B, -N, -R, -P, -P, -P, -P, -P, -P, -P, -P,
            E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,
            E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,  E,
            P,  P,  P,  P,  P,  P,  P,  P,  R,  N,  B,  Q,  K,  B,  N,  R };

    memcpy(board, start, sizeof(start));
}

// TODO: Implement algebaric notation
/** Asks for move, validates move and does move if valid
 *
 * \param turn_no Turn number, is increased between every `do_turn` elsewhere
 * \param board   Pointer to list of tiles representing board state
 * */
void do_turn(int turn_no, tile_t board[BOARD_SIZE])
{
    char input[3] = { 0 };

    int from = -1, to = -1, tmp;

    printf("\nPlayer %i, your turn to move", 1 + turn_no % 2);

    /* temporary and ugly solution - read from and to */
    while (from == -1 || to == -1) {
        from = -1;
        to   = -1;

        printf("\nMove piece\nfrom: ");
        scanf(" %2s", input);

        tmp = input_to_index(input);

        if (tmp == -1)
            continue;

        from = tmp;

        printf("\nto: ");
        scanf(" %2s", input);
        tmp = input_to_index(input);

        if (tmp == -1)
            continue;

        to = tmp;

        if (! move_ok(board, from, to, turn_no % 2 ? BLACK : WHITE)) {
            from = -1;
            continue;
        }
    }

    board[to]   = board[from];
    board[from] = E;
}

/**
 * Translates A1, 3B etc. to the 1D index of the board
 *
 *	\param input string of length 2 representing tile, e.g. "A3"
 *
 * */
index_t input_to_index(char input[2])
{
    int x = -1, y = -1, c;

    for (int i = 0; i < 2; i++) {
        c = input[i];

        if (isalpha(c))
            c = toupper(input[0]);

        if ('A' <= c && c <= 'H')
            x = c - 'A';
        else if ('1' <= c && c <= '8')
            y = c - '1';
    }

    if (x != -1 && y != -1)
        return 8 * (7 - y) + x;
    else
        return -1;
}

