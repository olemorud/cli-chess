
#include <ctype.h>   /* isalpha, isdigit ... */
#include <locale.h>  /* setlocale */
#include <stdbool.h> /* true, false */
#include <stdint.h>  /* int32_t */
#include <stdio.h>   /* printf, scanf */
#include <string.h>  /* memcpy */

/** Type representing piece/tile on a chessboard */
typedef int32_t tile_t;

/** Type representing index of a chessboard tile */
typedef ssize_t index_t;

#define ROW ((index_t)8)
#define COL ((index_t)1)

#define BOARD_SIZE ((index_t)(8 * 8))

#define E ((tile_t)0) ///< empty tile
#define K ((tile_t)1) ///< king
#define Q ((tile_t)2) ///< queen
#define R ((tile_t)3) ///< rook
#define B ((tile_t)4) ///< bishop
#define N ((tile_t)5) ///< knight
#define P ((tile_t)6) ///< pawn

/** Set background to dark blue */
#define BG_DARKBLUE()  setcolor(0, 100, 100, 150)

/** Set background to light blue */
#define BG_LIGHTBLUE() setcolor(0, 150, 150, 200)

/** Set foreground to black */
#define FG_BLACK()     setcolor(1, 0, 0, 0)

/** Set foreground to white */
#define FG_WHITE()     setcolor(1, 0xff, 0xff, 0xff)

#define WHITE 1
#define BLACK -1

/** 0x2659 == ♙ */
#define UNICODE_CHESS_SYMBOL 0x2659

bool bishop_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool cardinal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool diagonal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool king_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool knight_move_ok(index_t from, index_t to);
bool move_ok(const tile_t board[BOARD_SIZE],
             index_t      from,
             index_t      to,
             int          player);
bool pawn_move_ok(const tile_t board[BOARD_SIZE],
                  index_t      from,
                  index_t      to,
                  int          player);
bool queen_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool rook_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool tile_empty(tile_t t);
index_t abs_pos(index_t p);
index_t column(index_t i);
index_t row(index_t i);
index_t get_piece(char input[2]);
tile_t  abs_tile(tile_t t);
void    do_turn(int turn_no, tile_t board[BOARD_SIZE]);
void    init_board(tile_t board[BOARD_SIZE]);
void    print_board(const tile_t board[BOARD_SIZE]);
void    setcolor(int mode, int r, int g, int b);

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
void setcolor(const int mode, const int r, const int g, const int b)
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

        tmp = get_piece(input);

        if (tmp == -1)
            continue;

        from = tmp;

        printf("\nto: ");
        scanf(" %2s", input);
        tmp = get_piece(input);

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
index_t get_piece(char input[2])
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

/**
 * Returns the absolute value of an index_t value
 *
 * \param p positive or negative index_t
 */
index_t abs_pos(index_t p)
{
    if (p < 0)
        return -1 * p;

    return p;
}

/**
 * Returns the absolute value of a tile_t value
 *
 * \param t positive or negative tile_t
 * */
tile_t abs_tile(tile_t t)
{
    if (t < 0)
        return -1 * t;

    return t;
}

/**
 * Returns true if tile is empty, false otherwise
 *
 *	\param t tile to check if empty
 * */
bool tile_empty(tile_t t)
{
    return t == E;
}

/**
 * Returns row number of 1D board index
 *
 * \param i index to get row number of
 * */
index_t row(index_t i)
{
    return i / ROW;
}

/**
 * Returns column number of board index
 *
 * \param i index to get column number of
 * */
index_t column(index_t i)
{
    return i % ROW;
}

/**
 * Returns true if a and b are tiles of opposite player, false otherwise
 *
 * \param a Tile to compare
 * \param b Tile to compare it with
 * */
bool opposite_color(tile_t a, tile_t b)
{
    return a * b < 0;
}

/**
 * Returns true if a and b are pieces of the same color
 *
 * \param a Tile to compare
 * \param b Tile to compare it with
 * */
bool same_color(tile_t a, tile_t b)
{
    return a * b > 0;
}

/**
 * Returns true if a move is valid, false otherwise
 *
 * \param board  Pointer to list of tiles representing board state
 * \param from   Tile to move piece from
 * \param to     Tile to move piece to
 * \param player The current player to move
 * */
bool move_ok(const tile_t board[BOARD_SIZE],
             index_t      from,
             index_t      to,
             const int    player)
{
    /* player must own piece it moves */
    if (board[from] * player < 0) {
        printf("\nYou do not own this piece");
        return false;
    }

    if (tile_empty(board[from])) {
        printf("Can't move from empty tile");
        return false;
    }

    /* player can't take own pieces or move piece onto itself*/
    if (same_color(board[from], board[to])) {
        printf("\nYou can't take your own pieces");
        return false;
    }

    /* check piece specific moves */
    switch (abs_tile(board[from])) {
    case P:
        return pawn_move_ok(board, from, to, player);

    case B:
        return bishop_move_ok(board, from, to);

    case R:
        return rook_move_ok(board, from, to);

    case N:
        return knight_move_ok(from, to);

    case K:
        return king_move_ok(board, from, to);

    case Q:
        return queen_move_ok(board, from, to);
    }

    return false;
}

/**
 * Returns true if move is a valid pawn move
 *
 *  \param board  Array of tiles representing chess board state
 *  \param from   Index of board piece starts at
 *  \param to     Index of board piece wants to move to
 *  \param player Player to move
 */
bool pawn_move_ok(const tile_t board[BOARD_SIZE],
                  index_t      from,
                  index_t      to,
                  int          player)
{
    const index_t diff = (to - from) * -player;

    switch (diff) {
    default:
        return false;

    case ROW: /* single move */
        return tile_empty(board[to]);

    case ROW - 1:
    case ROW + 1: /* diagonal attack */
        return opposite_color(board[to], board[from]);

    case 2 * ROW: /* double move */
        return tile_empty(board[to]) && tile_empty(board[from - ROW * player])
            && (row(from) == 1 || row(from) == 6);
    }
}

/**
 * Returns true if `to` is on a diagonal line of `from`, false otherwise
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool diagonal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    const index_t col_diff = column(to) - column(from);
    const index_t row_diff = row(to) - row(from);

    const index_t x_step = col_diff / abs_pos(col_diff);
    const index_t y_step = ROW * row_diff / abs_pos(row_diff);
    const index_t step   = x_step + y_step;

    if (abs_pos(col_diff) != abs_pos(row_diff)) {
        printf("\nNot a diagonal move");
        return false;
    }

    for (index_t p = from + step; p != to; p += step) {
        if (! tile_empty(board[p])) {
            printf("\nCan't jump over pieces");
            return false;
        }
    }

    return true;
}

/**
 * Returns true if index `to` is on a cardinal line of `from`, false otherwise
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool cardinal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    const index_t col_diff = column(to) - column(from);
    const index_t row_diff = row(to) - row(from);

    if (row_diff > 0 && col_diff > 0)
        printf("Must move in a straight line");

    index_t step;

    if (row_diff)
        step = ROW * row_diff / abs_pos(row_diff);
    else
        step = col_diff / abs_pos(col_diff);

    for (index_t p = from + step; p != to; p += step) {
        if (! tile_empty(board[p])) {
            printf("\ncan't jump over pieces");
            return false;
        }
    }

    return true;
}

/**
 * Returns true if move is a valid bishop move
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool bishop_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    return diagonal_move_ok(board, from, to);
}

/**
 * Returns true if move is a valid rook move
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool rook_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    return cardinal_move_ok(board, from, to);
}

/**
 * Returns true if move is a valid knight move
 *
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool knight_move_ok(index_t from, index_t to)
{
    const index_t abs_col_diff = abs_pos(column(to) - column(from));
    const index_t abs_row_diff = abs_pos(row(to) - row(from));

    return (abs_col_diff == 1 && abs_row_diff == 2)
        || (abs_col_diff == 2 && abs_row_diff == 1);
}

/**
 * Returns true if move is a valid king move
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool king_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    const index_t abs_col_diff = abs_pos(column(to) - column(from));
    const index_t abs_row_diff = abs_pos(row(to) - row(from));

    (void)board;

    return abs_col_diff <= 1 && abs_row_diff <= 1;
}

/**
 * Returns true if move is a valid queen move
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 */
bool queen_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    return diagonal_move_ok(board, from, to)
        || cardinal_move_ok(board, from, to);
}
