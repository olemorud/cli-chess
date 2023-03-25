
#include "common.h"
#include "graphics.h"
#include "pieces.h"

#include <ctype.h>  /* isalpha, isdigit ... */
#include <locale.h> /* setlocale */
#include <stdint.h> /* uint64_t */
#include <stdio.h>  /* printf, scanf */
#include <string.h> /* memcpy */

#define CLEAR_SCREEN() printf("\033[2J")

int     do_turn(int turn_no, tile_t board[BOARD_SIZE]);
void    init_board(tile_t board[BOARD_SIZE]);
index_t input_to_index(char input[2]);

int main()
{
    setlocale(LC_ALL, "C.UTF-8");

    tile_t board[BOARD_SIZE] = { 0 };

    init_board(board);

    int turn = 0;

    while (true) {
        CLEAR_SCREEN();
        print_board(board);

        // turn increments on valid inputs,
        turn += do_turn(turn, board);
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
 * \param turn_no Turn number
 * \param board   Pointer to list of tiles representing board state
 *
 * \return (int)1 if successful, (int)0 if invalid input from player
 * */
int do_turn(int turn_no, tile_t board[BOARD_SIZE])
{
    char input[3] = { 0 };

    int from = -1, to = -1;

    printf("\nPlayer %i, your turn to move", 1 + turn_no % 2);
    printf("\nMove piece\nfrom: ");

    scanf(" %2s", input);

    from = input_to_index(input);

    if (from == -1)
        return 0;

    printf("\nto: ");
    scanf(" %2s", input);

    to = input_to_index(input);

    if (to == -1)
        return 0;

    if (! move_ok(board, from, to, turn_no % 2 ? BLACK : WHITE))
        return 0;

    board[to]   = board[from];
    board[from] = E;

    /* increment to make it next turn */
    return 1;
}

/**
 * Translates A1, 3B etc. to the 1D index of the board
 *
 *	\param input string of length 2 representing tile, e.g. "A3"
 *
 *	\return index value on valid input, -1 on invalid input
 *
 * */
index_t input_to_index(char input[2])
{
    int x = -1, y = -1;

    for (int i = 0; i < 2; i++) {
        const char c = input[i];

        const char v = isalpha(c) ? toupper(c) : c;

        if ('A' <= v && v <= 'H')
            x = v - 'A';
        else if ('1' <= v && v <= '8')
            y = v - '1';
    }

    if (x != -1 && y != -1)
        return 8 * (7 - y) + x;
    else
        return -1;
}
