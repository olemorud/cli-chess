
#include "graphics.h"
#include "common.h"
#include "util.h"

#include <stdio.h>

/** Set background to dark blue */
#define BG_DARKBLUE()  setcolor(0, 100, 100, 150)

/** Set background to light blue */
#define BG_LIGHTBLUE() setcolor(0, 150, 150, 200)

/** Set foreground to black */
#define FG_BLACK()     setcolor(1, 0, 0, 0)

/** Set foreground to white */
#define FG_WHITE()     setcolor(1, 0xff, 0xff, 0xff)

/** 0x2659 == ♙ */
#define UNICODE_CHESS_SYMBOL 0x2659

static inline void setcolor(const int mode, const int r, const int g, const int b);

/**
 * Sets the foreground or background color for subsequent writes.
 *
 * Uses Select Graphic Renditions (SGR) to set the color of the terminal output.
 * See https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit for more details.
 *
 * \param mode 0 - change background, 1 - change foreground, 2 - reset colors
 * \param r    amount of red (0 to 255)
 * \param b    amount of blue (0 to 255)
 * \param g    amount of green (0 to 255)
 */
static inline void setcolor(const int mode, const int r, const int g, const int b)
{
    if (mode == 2)
        printf("\033[0m");
    else
        printf("\033[%i;2;%i;%i;%im", mode ? 38 : 48, r, g, b);
};

/**
 * Prints the board
 *
 * Uses unicode symbols and ANSI escape features to print a chessboard on the
 * display.
 *
 * \param board A pointer to a list of tiles representing the board state
 *
 * */
void print_board(const tile_t board[BOARD_SIZE])
{
    /* https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode

       The unicode symbols for the pieces are calculated from adding
           0x2653 (#define'd as UNICODE_CHESS_SYMBOL) with the piece value. */

    for (size_t i = 0; i < 8; i++) {
        printf("\n %zu ", 8 - i); // number coordinates

        for (size_t j = 0; j < 8; j++) {
            tile_t t = board[i * 8 + j];

            if ((i + j) % 2)
                BG_DARKBLUE();
            else
                BG_LIGHTBLUE();

            if (tile_empty(t)) {
                printf("  ");
                continue;
            }

            if (t > 0)
                FG_WHITE();
            else
                FG_BLACK();

            printf("%lc ", UNICODE_CHESS_SYMBOL + abs_tile(t));
        }

        setcolor(2, 0, 0, 0); // reset text attributes
    }

    /* horizontal letter coordinates */
    printf("\n  ");

    for (int i = 0; i < 8; i++)
        printf(" %c", 'a' + i);
}
