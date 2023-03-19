
#include <ctype.h>   /* isalpha, isdigit ... */
#include <locale.h>  /* setlocale */
#include <stdbool.h> /* true, false */
#include <stdint.h>  /* int32_t */
#include <stdio.h>   /* printf, scanf */
#include <string.h>  /* memcpy */

typedef int32_t tile_t;
typedef ssize_t index_t;

#define ROW ((index_t)8)
#define COL ((index_t)1)

#define BOARD_SIZE (8 * 8 * sizeof(tile_t))

#define E ((tile_t)0) /* empty */
#define K ((tile_t)1) /* king */
#define Q ((tile_t)2) /* queen  */
#define R ((tile_t)3) /* rook */
#define B ((tile_t)4) /* bishop */
#define N ((tile_t)5) /* knight */
#define P ((tile_t)6) /* pawn */

#define BG_DARKBLUE()  setcolor(0, 100, 100, 150)
#define BG_LIGHTBLUE() setcolor(0, 150, 150, 200)
#define FG_BLACK()     setcolor(1, 0, 0, 0)
#define FG_WHITE()     setcolor(1, 0xff, 0xff, 0xff)

#define WHITE 1
#define BLACK -1

#define UNICODE_CHESS_SYMBOL 0x2659

bool bishop_move_ok(const tile_t *board, index_t from, index_t to);
bool cardinal_move_ok(const tile_t *board, index_t from, index_t to);
bool diagonal_move_ok(const tile_t *board, index_t from, index_t to);
bool king_move_ok(const tile_t *board, index_t from, index_t to);
bool knight_move_ok(index_t from, index_t to);
bool move_ok(tile_t *board, index_t from, index_t to, int player);
bool pawn_move_ok(const tile_t *board, index_t from, index_t to, int direction);
bool queen_move_ok(const tile_t *board, index_t from, index_t to);
bool rook_move_ok(const tile_t *board, index_t from, index_t to);
bool tile_empty(tile_t t);
index_t abs_pos(index_t p);
index_t column(index_t t);
index_t row(index_t t);
int     get_piece(char *input);
tile_t  abs_tile(tile_t t);
void    do_turn(int turn_no, tile_t *board);
void    init_board(tile_t *board);
void    print_board(tile_t *board);
void    setcolor(int mode, int r, int g, int b);

int main()
{
    int turn = 0;

    setlocale(LC_ALL, "C.UTF-8");

    tile_t board[8 * 8] = { 0 };

    init_board(board);

    while (1) {
        print_board(board);
        do_turn(turn++, board);
    }

    return 0;
}

/*
  Sets the foreground or background color for subsequent writes.
  Modes:
   0: change background
   1: change foreground
   2: reset colors
 */
void setcolor(const int mode, const int r, const int g, const int b)
{
    if (mode == 2)
        printf("\033[0m");
    else
        printf("\033[%i;2;%i;%i;%im", mode ? 38 : 48, r, g, b);
};

/* Prints the board */
void print_board(tile_t *board)
{
    /* https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode

            The unicode symbol is calculated from adding 0x2653 with the
            piece value. */

    for (size_t i = 0; i < 8; i++) {
        printf("\n %zu ", 8 - i); // print number coordinates on y-axis

        for (size_t j = 0; j < 8; j++) {
            tile_t p = board[i * 8 + j];

            // Make tile dark and light
            if ((i + j) % 2)
                BG_DARKBLUE();
            else
                BG_LIGHTBLUE();

            // Print empty space and do nothing if tile is empty
            if (tile_empty(p)) {
                printf("  ");
                continue;
            }

            // Set piece color
            if (p > 0)
                FG_WHITE();
            else
                FG_BLACK();

            printf("%lc ", UNICODE_CHESS_SYMBOL + abs_tile(p));
        }

        setcolor(2, 0, 0, 0); // reset text attributes
    }

    // Print horizontal letter coordinates
    printf("\n  ");

    for (int i = 0; i < 8; i++)
        printf(" %c", 'a' + i);
}

/*
 * Resets/inits the board
 */
void init_board(tile_t *board)
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
/* Get move, check move and log move for turn <turn_no> */
void do_turn(int turn_no, tile_t *board)
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

/* Translates A1, 3B etc. to the 1D index of the board */
int get_piece(char *input)
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

index_t abs_pos(index_t p)
{
    if (p < 0)
        return -1 * p;

    return p;
}

tile_t abs_tile(tile_t t)
{
    if (t < 0)
        return -1 * t;

    return t;
}

/* Returns true if tile is empty */
bool tile_empty(tile_t t)
{
    return t == E;
}

/* Returns row number of board index */
index_t row(index_t t)
{
    return t / ROW;
}

/* Returns column number of board index */
index_t column(index_t t)
{
    return t % ROW;
}

/* Returns true if a and b are tiles of opposite player */
bool opposite_color(tile_t a, tile_t b)
{
    return a * b < 0;
}

/* Returns true if a and b are pieces of the same color */
bool same_color(tile_t a, tile_t b)
{
    return a * b > 0;
}

/* Returns true if a move is valid, false otherwise */
bool move_ok(tile_t *board, index_t from, index_t to, const int player)
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

/* Returns true if move is a valid pawn move
    board     - array of tiles representing chess board state
    from      - index of board piece starts at
    to        - index of board piece wants to move to
    direction - pawns movement direction */
bool pawn_move_ok(const tile_t *board, index_t from, index_t to, int direction)
{
    const index_t diff = (to - from) * -direction;

    switch (diff) {
    default:
        return false;

    case ROW: /* single move */
        return tile_empty(board[to]);

    case ROW - 1:
    case ROW + 1: /* diagonal attack */
        return opposite_color(board[to], board[from]);

    case 2 * ROW: /* double move */
        return tile_empty(board[to])
            && tile_empty(board[from - ROW * direction])
            && (row(from) == 1 || row(from) == 6);
    }
}

/* Returns true if `to` is on a diagonal line of `from`
    board - array of tiles representing chess board state
    from  - index of board piece is at
    to    - index of board piece tries to move to */
bool diagonal_move_ok(const tile_t *board, index_t from, index_t to)
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

/* Returns true if `to` is in a cardinal line of `from`
    board - array of tiles representing chess board state
    from  - index of board piece is at
    to    - index of board piece tries to move to */
bool cardinal_move_ok(const tile_t *board, index_t from, index_t to)
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

/* Returns true if move is a valid bishop move
    board - array of tiles representing chess board state
    from  - index of board bishop is at
    to    - index of board bishop wants to move to */
bool bishop_move_ok(const tile_t *board, index_t from, index_t to)
{
    return diagonal_move_ok(board, from, to);
}

/* Returns true if move is a valid rook move
    board - array of tiles representing chess board state
    from  - index of board rook is at
    to    - index of board rook wants to move to */
bool rook_move_ok(const tile_t *board, index_t from, index_t to)
{
    return cardinal_move_ok(board, from, to);
}

/* Returns true if move is a valid knight move
    board - array of tiles representing chess board state
    from  - index of board knight is at
    to    - index of board knight wants to move to */
bool knight_move_ok(index_t from, index_t to)
{
    const index_t abs_col_diff = abs_pos(column(to) - column(from));
    const index_t abs_row_diff = abs_pos(row(to) - row(from));

    return (abs_col_diff == 1 && abs_row_diff == 2)
        || (abs_col_diff == 2 && abs_row_diff == 1);
}

/* Returns true if move is a valid king move
    board - array of tiles representing chess board state
    from  - index of board king is at
    to    - index of board king wants to move to */
bool king_move_ok(const tile_t *board, index_t from, index_t to)
{
    const index_t abs_col_diff = abs_pos(column(to) - column(from));
    const index_t abs_row_diff = abs_pos(row(to) - row(from));

    (void)board;

    return abs_col_diff <= 1 && abs_row_diff <= 1;
}

/* Returns true if move is a valid queen move
    board - array of tiles representing chess board state
    from  - index of board queen is at
    to    - index of board queen wants to move to */
bool queen_move_ok(const tile_t *board, index_t from, index_t to)
{
    return diagonal_move_ok(board, from, to)
        || cardinal_move_ok(board, from, to);
}
