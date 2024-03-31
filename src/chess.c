
#include <unistd.h>

#include "cool_assert.h"
#include <ctype.h>   /* isalpha, isdigit ... */
#include <locale.h>  /* setlocale */
#include <math.h>
#include <signal.h>
#include <stdbool.h> /* true, false, bool */
#include <stddef.h>  /* ptrdiff_t */
#include <stdint.h>  /* int32_t */
#include <stdio.h>   /* printf, scanf */
#include <stdlib.h>

#define MAX_DEPTH 4
#define CHECKMATE_SCORE 100000

#define RANK       ((index_t)8)
#define COL        ((index_t)1)

typedef int8_t    piece_t;
typedef ptrdiff_t index_t;
typedef uint64_t  bitmap_t;

static const char * const bool_str[] = {"true", "false"};

enum tile {
    A1,   B1,   C1,   D1,   E1,   F1,   G1,   H1,
    A2,   B2,   C2,   D2,   E2,   F2,   G2,   H2,
    A3,   B3,   C3,   D3,   E3,   F3,   G3,   H3,
    A4,   B4,   C4,   D4,   E4,   F4,   G4,   H4,
    A5,   B5,   C5,   D5,   E5,   F5,   G5,   H5,
    A6,   B6,   C6,   D6,   E6,   F6,   G6,   H6,
    A7,   B7,   C7,   D7,   E7,   F7,   G7,   H7,
    A8,   B8,   C8,   D8,   E8,   F8,   G8,   H8,
    BOARD_SIZE,
};

static const char * const tile_str[BOARD_SIZE] = {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
};

enum board_rank {
    RANK_1 = 0,
    RANK_2 = 8,
    RANK_3 = 16,
    RANK_4 = 24,
    RANK_5 = 32,
    RANK_6 = 40,
    RANK_7 = 48,
    RANK_8 = 56,
};

enum board_file {
    FILE_A = 0,
    FILE_B = 1,
    FILE_C = 2,
    FILE_D = 3,
    FILE_E = 4,
    FILE_F = 5,
    FILE_G = 6,
    FILE_H = 7,
};

enum color {
    BLACK = -1,
    WHITE = 1,
};

static const char * const color_str[] = {
    "WHITE",
    "BLACK"
};

static const char * const color_str_lower[] = {
    "white",
    "black"
};

static const char * const color_str_capitalized[] = {
    "White",
    "Black"
};

typedef piece_t Board[BOARD_SIZE];

enum chess_piece {
    EMPTY  = 0,
    KING   = 1,
    QUEEN  = 2,
    ROOK   = 3,
    BISHOP = 4,
    KNIGHT = 5,
    PAWN   = 6,
    PIECE_COUNT,
};

static const double piece_value[] = {
    [EMPTY]  = 0,
    [PAWN]   = 1,
    [BISHOP] = 3,
    [KNIGHT] = 3,
    [ROOK]   = 5,
    [QUEEN]  = 9,
    [KING]   = 10,
};

static const double piece_position_bonus[PIECE_COUNT][BOARD_SIZE] = {
    [EMPTY] = {0},
    [PAWN] = {
       /*       A    B    C    D    E    F    G    H    */
       /* 1 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 2 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 3 */ 1.0, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.0, 
       /* 4 */ 1.0, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.0, 
       /* 5 */ 1.2, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.2, 
       /* 6 */ 1.7, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.7, 
       /* 7 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 8 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
    [BISHOP] = {
       /*       A    B    C    D    E    F    G    H    */
       /* 1 */ 1.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.2, 
       /* 2 */ 1.2, 1.2, 1.0, 1.0, 1.0, 1.2, 1.2, 1.0, 
       /* 3 */ 1.0, 1.2, 1.2, 1.0, 1.2, 1.2, 1.0, 1.0, 
       /* 4 */ 1.0, 1.0, 1.2, 1.2, 1.2, 1.0, 1.0, 1.0, 
       /* 5 */ 1.0, 1.0, 1.2, 1.2, 1.2, 1.0, 1.0, 1.0, 
       /* 6 */ 1.0, 1.2, 1.2, 1.0, 1.2, 1.2, 1.0, 1.0, 
       /* 7 */ 1.2, 1.2, 1.0, 1.0, 1.0, 1.2, 1.2, 1.0, 
       /* 8 */ 1.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.2, 
    },
    [KNIGHT] = {
       /*       A    B    C    D    E    F    G    H    */
       /* 1 */ 0.5, 0.7, 0.8, 0.8, 0.8, 0.8, 0.7, 0.5, 
       /* 2 */ 0.6, 0.7, 0.9, 0.9, 0.9, 0.9, 0.7, 0.6, 
       /* 3 */ 0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
       /* 4 */ 0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
       /* 5 */ 0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
       /* 6 */ 0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
       /* 7 */ 0.6, 0.7, 0.9, 0.9, 0.9, 0.9, 0.7, 0.6, 
       /* 8 */ 0.5, 0.7, 0.8, 0.8, 0.8, 0.8, 0.7, 0.5, 
    },
    [ROOK] = {
       /*       A    B    C    D    E    F    G    H    */
       /* 1 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 2 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 3 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 4 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 5 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 6 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 7 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 8 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
    [QUEEN] = {
       /*       A    B    C    D    E    F    G    H    */
       /* 1 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 2 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 3 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 4 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 5 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 6 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 7 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
       /* 8 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
    [KING] = {
       /*       A    B    C    D    E    F    G    H    */
       /* 1 */ 1.1, 1.1, 1.1, 1.0, 1.0, 1.0, 1.15, 1.15, 
       /* 2 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
       /* 3 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
       /* 4 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
       /* 5 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
       /* 6 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
       /* 7 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
       /* 8 */ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 
    },
};

enum game_state_attr {
    KING_POSITION  = (1<<6)-1, // mask of king position
    A_ROOK_TOUCHED = 1<<6,
    H_ROOK_TOUCHED = 1<<7,
    KING_TOUCHED   = 1<<8,
};

enum attr_color {
    ATTR_WHITE = 0,
    ATTR_BLACK = 1,
};

inline size_t attr_index(enum color color) {
    return color == WHITE ? ATTR_WHITE : ATTR_BLACK;
}

struct game_state {
    Board board;
    uint32_t attr[2];
    int last_pawn_double_move_file;
    int turns_without_captures;
    int turns;
    enum color player;
};
// hacky solution to pass game state to sigint handler
static struct game_state sigint_state_copy;

enum castle_type {
    CASTLE_KINGSIDE  = 1,
    CASTLE_QUEENSIDE = 2,
};

static inline bitmap_t bit(index_t i)
{
    return 1UL << i;
}

static inline bool friends(piece_t a, piece_t b)
{
    return a * b > 0;
}

static inline piece_t piece_abs(piece_t t)
{
    if (t < 0)
        return -t;
    return t;
}

static int signum(int t)
{
    if (t == 0)
        return 0;
    if (t >= 0)
        return 1;
    return -1;
}

static inline index_t rank(index_t i)
{
    return (i / RANK)*8;
}

static inline index_t file(index_t i)
{
    return i % RANK;
}

static inline bool enemies(piece_t a, piece_t b)
{
    return a * b < 0;
}

static inline piece_t piece_color(piece_t t)
{
    return (piece_t)signum(t);
}

static const char * piece_str(piece_t p) {
    static const char * const table[] = {
        [EMPTY]  = "EMPTY",
        [PAWN]   = "-PAWN",
        [BISHOP] = "-BISHOP",
        [KNIGHT] = "-KNIGHT",
        [ROOK]   = "-ROOK",
        [QUEEN]  = "-QUEEN",
        [KING]   = "-KING",
    };
    const size_t pa = piece_abs(p);
    return table[pa] + (piece_color(p) == WHITE ? 1 : 0);
}

static void paint_board(struct game_state* g, index_t highlight1, index_t highlight2)
{
    /* https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
       The unicode symbols for the pieces are calculated from adding
       0x2653 (#define'd as UNICODE_CHESS_SYMBOL) with the piece value. */
#define BG_RED()       printf("\033[48;2;150;150;0m")
#define BG_DARKBLUE()  printf("\033[48;2;100;100;150m")
#define BG_LIGHTBLUE() printf("\033[48;2;150;150;200m")
#define FG_BLACK()     printf("\033[38;2;0;0;0m")
#define FG_WHITE()     printf("\033[38;2;255;255;255m")
#define UNICODE_CHESS_SYMBOL 0x2659
    printf("%s's turn", g->player == WHITE ? "White" : "Black");
    for (index_t i = 7; i >= 0; i--) {
    //for (index_t i = 0; i < 8; i++) {
        printf("\n %ld ", i+1); // number coordinates
        for (index_t j = 0; j < 8; j++) {
            piece_t t = g->board[i * 8 + j];
            if (i*RANK+j == highlight1 || i*RANK+j == highlight2)
                BG_RED();
            else if ((i + j) % 2)
                BG_DARKBLUE();
            else
                BG_LIGHTBLUE();
            if (t == EMPTY) {
                printf("  ");
                continue;
            }
            if (t > 0)
                FG_WHITE();
            else
                FG_BLACK();
            printf("%lc ", UNICODE_CHESS_SYMBOL + piece_abs(t));
        }
        //setcolor(2, 0, 0, 0); // reset text attributes
        printf("\033[0m");
    }
    /* horizontal letter coordinates */
    printf("\n  ");
    for (int i = 0; i < 8; i++)
        printf(" %c", 'a' + i);

    printf("\n");
}

static void move(struct game_state* g, index_t from, index_t to)
{
    static_assert(WHITE == 1,  "`WHITE` must match direction of white pawns (1) for move() to work");
    static_assert(BLACK == -1, "`BLACK` must match direction of black pawns (-1) for move() to work");

    const int piece = piece_abs(g->board[from]);
    const enum color player = g->player;
    const int p = attr_index(player);

    g->turns  += 1;
    g->player *= -1;
    g->last_pawn_double_move_file = -1;

    if (from == A8 || to == A8) {
        g->attr[ATTR_BLACK] |= A_ROOK_TOUCHED;
    } else if (from == A1 || to == A1) {
        g->attr[ATTR_WHITE] |= A_ROOK_TOUCHED;
    } else if (from == H1 || to == H1) {
        g->attr[ATTR_WHITE] |= H_ROOK_TOUCHED;
    } else if (from == H8 || to == H8) {
        g->attr[ATTR_BLACK] |= H_ROOK_TOUCHED;
    }

    if (g->board[to] == EMPTY) {
        g->turns_without_captures += 1;
    } else {
        g->turns_without_captures = 0;
    }

    // castle
    if (piece == KING) {
        g->attr[p] &= ~KING_POSITION;
        g->attr[p] |= to;
        g->attr[p] |= KING_TOUCHED;

        // castling
        if (player == WHITE && to == G1) {
            g->board[F1] = ROOK;
            g->board[H1] = EMPTY;
        } else if (player == BLACK && to == G8) {
            g->board[F8] = -ROOK;
            g->board[H8] = EMPTY;
        } else if (player == WHITE && to == C1) {
            g->board[A1] = EMPTY;
            g->board[B1] = EMPTY;
            g->board[D1] = ROOK;
        } else if (player == BLACK && to == C8) {
            g->board[A8] = EMPTY;
            g->board[B8] = EMPTY;
            g->board[D8] = -ROOK;
        }
        g->board[to]   = g->board[from];
        g->board[from] = EMPTY;
        return;
    }
    // en passent
    else if (piece == PAWN) {
        if (g->last_pawn_double_move_file == file(to)) {
            g->board[to-RANK * player] = EMPTY;
        }
        if (to - from == 2*RANK * player) {
            g->last_pawn_double_move_file = file(to);
        }

        if (rank(to) == RANK_1 || rank(to) == RANK_8) {
            // promotion, TODO: implement other promotions
            g->board[to] = player * QUEEN;
        } else {
            g->board[to] = g->board[from];
        }
        g->board[from] = EMPTY;
    } else {
        g->board[to]   = g->board[from];
        g->board[from] = EMPTY;
    }
}

static bitmap_t pawn_threatmap(struct game_state* g, index_t index)
{
    const index_t left  = bit(index + RANK*g->player - 1);
    const index_t right = bit(index + RANK*g->player + 1);

    if (file(index) == FILE_A)
        return right;

    if (file(index) == FILE_H)
        return left;

    return left | right;
}

static bitmap_t diagonal_threatmap(struct game_state* g, index_t index)
{
    bitmap_t threatened = 0;

    //index_t directions[] = { RANK+1, RANK-1, -RANK+1, -RANK-1 };

    for (index_t i = index+RANK+1; i < BOARD_SIZE && file(i-1) != 7; i += RANK+1) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    for (index_t i = index+RANK-1; i < BOARD_SIZE && file(i+1) != 0; i += RANK-1) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    for (index_t i = index-RANK+1; i >= 0 && file(i-1) != 7; i += -RANK+1) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    for (index_t i = index-RANK-1; i >= 0 && file(i+1) != 0; i += -RANK-1) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    return threatened;
}

static bitmap_t cardinal_threatmap(struct game_state* g, index_t index)
{
    bitmap_t threatened = 0;

    for (index_t i = index+RANK; i < BOARD_SIZE; i += RANK) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY)
            break;
    }
    for (index_t i = index-RANK; i >= 0; i -= RANK) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY)
            break;
    }
    for (index_t i = index+1; i < BOARD_SIZE && file(i) != 0; i++) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY)
            break;
    }
    for (index_t i = index-1; i > 0 && file(i) != 7; i--) {
        threatened |= bit(i);
        if (g->board[i] != EMPTY)
            break;
    }

    return threatened;
}

static inline bitmap_t bishop_threatmap(struct game_state* g, index_t index)
{
    return diagonal_threatmap(g, index);
}

static inline bitmap_t rook_threatmap(struct game_state* g, index_t index)
{
    return cardinal_threatmap(g, index);
}

static bitmap_t knight_threatmap(index_t index)
{
    bitmap_t threatened = 0L;

    //clang-format off
    index_t knight_wheel[8*2] = {
     /*  x,   y  */
         1,   2*RANK,
         1,  -2*RANK,
        -1,   2*RANK,
        -1,  -2*RANK,
         2,   1*RANK,
         2,  -1*RANK,
        -2,   1*RANK,
        -2,  -1*RANK
    };
    // clang-format on
    
    for (size_t i = 0; i < sizeof knight_wheel / sizeof knight_wheel[0]; i += 2) {
        if (file(index) + knight_wheel[i] < FILE_A
         || file(index) + knight_wheel[i] > FILE_H
         || rank(index) + knight_wheel[i+1] < RANK_1
         || rank(index) + knight_wheel[i+1] > RANK_8)
            continue;

        threatened |= bit(index + knight_wheel[i] + knight_wheel[i+1]);
    }

    return threatened;
}

static bitmap_t king_threatmap(index_t index)
{
    // I fucking hate this function so much
    if (rank(index) == RANK_1) {
        if (file(index) == FILE_A) {
            return bit(index+1) | bit(index+RANK+1) | bit (index+RANK);
        }
        else if (file(index) == FILE_H) {
            return bit(index-1) | bit(index+RANK-1) | bit (index+RANK);
        }
        else {
            return bit(index-1) | bit(index+1) | bit(index+RANK-1) | bit(index+RANK) | bit(index+RANK+1);
        }
    }
    if (rank(index) == RANK_8) {
        if (file(index) == FILE_A) {
            return bit(index+1) | bit(index-RANK+1) | bit (index-RANK);
        }
        else if (file(index) == FILE_H) {
            return bit(index-1) | bit(index-RANK-1) | bit (index-RANK);
        }
        else {
            return bit(index-1) | bit(index+1) | bit(index-RANK-1) | bit(index-RANK) | bit(index-RANK+1);
        }
    } else {
        if (file(index) == FILE_A) {
            return bit(index-RANK) | bit(index-RANK+1) | bit(index+1) | bit(index+RANK+1) | bit(index+RANK);
        }
        else if (file(index) == FILE_H) {
            return bit(index-RANK) | bit(index-RANK-1) | bit(index-1) | bit(index+RANK-1) | bit(index+RANK);
        } else {
            return bit(index-RANK-1) | bit(index-RANK) | bit(index-RANK+1)
                 | bit(index-1)                        | bit(index+1)
                 | bit(index+RANK-1) | bit(index+RANK) | bit(index+RANK+1);
        }
    }
}

static inline bitmap_t queen_threatmap(struct game_state* g, index_t index)
{   
    return diagonal_threatmap(g, index) | cardinal_threatmap(g, index);
}

static void print_threatmap(bitmap_t threatmap)
{
    for (ssize_t i=7; i >= 0; i--) {
        printf("\n %ld ", i+1); // number coordinates
        for (size_t j = 0; j < 8; j++) {
            fputc(' ', stdout);
            if (threatmap & bit(i*RANK+j))
                fputc('x', stdout);
            else
                fputc('-', stdout);
        }
    }
    /* horizontal letter coordinates */
    printf("\n   ");
    for (int i = 0; i < 8; i++)
        printf(" %c", 'a' + i);
    fputc('\n', stdout);
}

static bitmap_t piece_threatmap(struct game_state* g, index_t index)
{
    switch (piece_abs(g->board[index])) {
    case EMPTY:
        return 0L;
    case PAWN:
        return pawn_threatmap(g, index);
    case BISHOP:
        return bishop_threatmap(g, index);
    case ROOK:
        return rook_threatmap(g, index);
    case KNIGHT:
        return knight_threatmap(index);
    case KING:
        return king_threatmap(index);
    case QUEEN:
        return queen_threatmap(g, index);
    default:
        return 0L;
    }
}

static bitmap_t threatmap(struct game_state* g, enum color attacker)
{
    enum color p = g->player;
    g->player = attacker;
    bitmap_t t = 0;
    for(index_t i = 0; i < BOARD_SIZE; i++) {
        if (friends(g->board[i], attacker)) {
            t |= piece_threatmap(g, i);
        }
    }
    g->player = p;
    return t;
}

static bool pawn_move_ok(struct game_state* g, index_t from, index_t to)
{
    //printf("checking pawn move for %s\n", g->player == WHITE ? "WHITE" : "BLACK");
    const index_t diff = (to - from) * g->player;
    const index_t starting_rank = g->player == WHITE ? RANK_2 : RANK_7;

    switch (diff) {
    case RANK: /* single move */
        return g->board[to] == EMPTY;

    case RANK - COL: /* diagonal attack */
    case RANK + COL:
        if ((file(from) == FILE_A && file(to) == FILE_H)
         || (file(from) == FILE_H && file(to) == FILE_A)
        ) {
            return false;
        } else if (file(to) == g->last_pawn_double_move_file
                && rank(from) == (g->player == WHITE ? RANK_5 : RANK_4)
        ) {
            return true;
        } else {
            return enemies(g->board[to], g->board[from]);
        }

    case 2 * RANK: /* double move */
        return g->board[to] == EMPTY
            && g->board[from + RANK*g->player] == EMPTY
            && rank(from) == starting_rank;

    default:
        return false;
    }
}

static bool is_check(struct game_state* g, enum color player)
{
    return bit(g->attr[attr_index(player)] & KING_POSITION) & threatmap(g, -player);
}

static bool castle_kingside_ok(struct game_state* g)
{
    if (is_check(g, g->player)) {
        return false;
    }
    const int p = attr_index(g->player);
    const int rank = g->player == WHITE ? RANK_1 : RANK_8;

    return !(g->attr[p] & H_ROOK_TOUCHED)
        && !(g->attr[p] & KING_TOUCHED)
        && !(threatmap(g, -g->player) & (bit(FILE_F + rank) | bit(FILE_G + rank)))
        && g->board[FILE_G + rank] == EMPTY
        && g->board[FILE_F + rank] == EMPTY;
}

static bool castle_queenside_ok(struct game_state* g)
{
    if (is_check(g, g->player)) {
        return false;
    }
    const int p = attr_index(g->player);
    const int rank = g->player == WHITE ? RANK_1 : RANK_8;
    return !(g->attr[p] & A_ROOK_TOUCHED)
        && !(g->attr[p] & KING_TOUCHED)
        && !(threatmap(g, -g->player) & (bit(FILE_C + rank) | bit(FILE_D + rank)))
        && g->board[FILE_B + rank] == EMPTY
        && g->board[FILE_C + rank] == EMPTY
        && g->board[FILE_D + rank] == EMPTY;
}

static bool king_move_ok(struct game_state* g, index_t from, index_t to)
{
    if (g->player == WHITE && from == E1) {
        if (to == G1) {
            return castle_kingside_ok(g);
        } else if (to == C1) {
            return castle_queenside_ok(g);
        }
    } else if (g->player == BLACK && from == E8) {
        if (to == G8) {
            return castle_kingside_ok(g);
        } else if (to == C8) {
            return castle_queenside_ok(g);
        }
    }
    return bit(to) & king_threatmap(from)
        && bit(to) & ~threatmap(g, -piece_color(g->board[from]));
}

static bool move_ok(struct game_state* g, index_t from, index_t to)
{
    //printf("checking move for %s\n", player_str[g->player]);
    /* Player must own piece it moves
       and a player can't capture their own pieces. */
    if (g->board[from] == EMPTY || enemies(g->player, g->board[from]) || friends(g->player, g->board[to])) {
        //printf("must own piece it moves and can't attack its own pieces\n");
        return false;
    }

    typeof(*g) restore = *g;
    move(g, from, to);
    bool check = is_check(g, -g->player);
    *g = restore;
    if (check) {
        //printf("move causes check!\n");
        return false;
    }

    switch (piece_abs(g->board[from])) {
    case EMPTY:
        //printf("can't move empty tile\n");
        return false;
    case PAWN:
        //printf("checking pawn move...\n");
        return pawn_move_ok(g, from, to);
    case KING:
        //printf("checking king move...\n");
        return king_move_ok(g, from, to);
    default:
        //printf("checking other move...\n");
        return bit(to) & piece_threatmap(g, from);
    }

    assert(false);
}

static bitmap_t valid_moves(struct game_state* g, index_t i)
{
    bitmap_t output = 0;
    for (index_t j=0; j<BOARD_SIZE; j++) {
        if (move_ok(g, i, j)) {
            output |= bit(j);
        }
    }
    return output;
}

static inline bool draw(struct game_state* g)
{
    // TODO: implement stalemate
    return g->turns_without_captures >= 50;
}

// TODO: fix this garbage
static bool checkmate(struct game_state* g)
{
    if (!is_check(g, g->player))
        return false;

    // TODO: avoid doubly nested for loop
    for (int i=0; i<BOARD_SIZE; i++) {
        for (int j=0; j<BOARD_SIZE; j++) {
            if (move_ok(g, i, j)) {
                return false;
            }
        }
    }

    return true;
}

static void dump_game_state(struct game_state* g)
{
    printf("\nstatic const struct game_state state = {");
    printf("\n    .board = {");
    int n;
    for (int i=0; i<8; i++) {
        printf("\n        ");
        n = 10;
        for (int j=0; j<8; j++) {
            n = printf("%s,", piece_str(g->board[i*RANK+j]));
            for (int k=0; k<10-n; k++) {
                printf(" ");
            }
        }
    }
    printf("\n    },");
    printf("\n    .attr = {");
    printf("\n        [0] = 0x%x,", g->attr[0]);
    printf("\n        [1] = 0x%x,", g->attr[1]);
    printf("\n    },");
    printf("\n    .last_pawn_double_move_file = %d,", g->last_pawn_double_move_file);
    printf("\n    .turns_without_captures = %d,", g->turns_without_captures);
    printf("\n    .turns = %d,", g->turns);
    printf("\n    .player = %s,", color_str[g->player]);
    printf("\n};");
    printf("\n");
}

static void game_init(struct game_state* g)
{
    // black pieces are prefixed by a minus (-)
    // clang-format off
#if 1
    static const struct game_state start = {
        .board = {
       /*       A        B        C        D        E        F        G        H    */
       /* 1 */  ROOK,    KNIGHT,  BISHOP,  QUEEN,   KING,    BISHOP,  KNIGHT,  ROOK,
       /* 2 */  PAWN,    PAWN,    PAWN,    PAWN,    PAWN,    PAWN,    PAWN,    PAWN,
       /* 3 */  EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,
       /* 4 */  EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,
       /* 5 */  EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,
       /* 6 */  EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,   EMPTY,
       /* 7 */  -PAWN,   -PAWN,   -PAWN,   -PAWN,   -PAWN,   -PAWN,   -PAWN,   -PAWN,
       /* 8 */  -ROOK,   -KNIGHT, -BISHOP, -QUEEN,  -KING,   -BISHOP, -KNIGHT, -ROOK,
        },
        .attr = {
            [0] = E1 & KING_POSITION,
            [1] = E8 & KING_POSITION,
        },
        .last_pawn_double_move_file = -1,
        .turns_without_captures = 0,
        .turns = 0,
        .player = WHITE,
    };
#else
    static const struct game_state start = {
    .board = {
        EMPTY,    EMPTY,    EMPTY,    EMPTY,    KING,     EMPTY,    EMPTY,    EMPTY,    
        -QUEEN,   EMPTY,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    
        PAWN,     EMPTY,    PAWN,     EMPTY,    EMPTY,    EMPTY,    EMPTY,    PAWN,     
        EMPTY,    PAWN,     EMPTY,    EMPTY,    BISHOP,   EMPTY,    EMPTY,    EMPTY,    
        EMPTY,    -PAWN,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    
        -PAWN,    EMPTY,    EMPTY,    ROOK,     EMPTY,    EMPTY,    EMPTY,    EMPTY,    
        -KING,    EMPTY,    EMPTY,    EMPTY,    EMPTY,    -KNIGHT,  ROOK,     EMPTY,    
        EMPTY,    EMPTY,    EMPTY,    EMPTY,    QUEEN,    EMPTY,    EMPTY,    EMPTY,    
    },
    .attr = {
        [0] = 0x1c4,
        [1] = 0x1f0,
    },
    .last_pawn_double_move_file = -1,
    .turns_without_captures = 2,
    .turns = 145,
    .player = BLACK,
};
#endif
    // clang-format on  

    *g = start;
    sigint_state_copy = *g;
}

static index_t input_to_index(char input[2])
{
    const int file = toupper(input[0])-'A';
    const int rank = RANK*(input[1] - '1');

    //printf("evaluated to index %d\n", file + rank);

    if (file < FILE_A || file > FILE_H || rank < RANK_1 || rank > RANK_8)
        return -1;

    return file + rank;
}

// TODO: Implement algebaric notation
static bool player_move(struct game_state* g)
{
    char input[3] = { 0 };

    int from = -1, to = -1;

    printf("\nMove piece\nfrom: ");
    scanf(" %2s", input);

    from = input_to_index(input);

    if (from == -1)
        return false;

    //print_threatmap(valid_moves(g, from));
    printf("\nto: ");
    scanf(" %2s", input);

    to = input_to_index(input);

    if (to == -1)
        return false;

    if (!move_ok(g, from, to))
        return false;

    move(g, from, to);

    return true;
}

static double heuristic(struct game_state* g, int depth)
{
    if (draw(g))
        return 0;

    if (checkmate(g))
        return g->player * -9999;

    double score = 0;
    for (index_t i=0; i<BOARD_SIZE; i++) {
        const piece_t piece  = g->board[i];
        const piece_t type = piece_abs(piece);
        score += piece_color(piece) * piece_value[type] * piece_position_bonus[type][(g->player == WHITE ? i : BOARD_SIZE-i-1)];
    }
    if (is_check(g, g->player)) {
        score += g->player * -1.0;
    }
    return score;
}

static void print_debug(struct game_state* g)
{
    for (int i=0; i<2; i++) {
        printf("%s:\n", color_str_capitalized[i]);
        printf("    A king touched: %s\n",        bool_str[!!(g->attr[i] & KING_TOUCHED)]);
        printf("    A rook touched: %s\n",        bool_str[!!(g->attr[i] & A_ROOK_TOUCHED)]);
        printf("    H rook touched: %s\n",        bool_str[!!(g->attr[i] & H_ROOK_TOUCHED)]);
        printf("    can king side castle: %s\n",  bool_str[castle_kingside_ok(g)]);
        printf("    can queen side castle: %s\n", bool_str[castle_queenside_ok(g)]);
        printf("    king pos: %s\n",              tile_str[g->attr[i] & KING_POSITION]);
        printf("    king pos: %u\n",              g->attr[i] & KING_POSITION);
        printf("    in check: %s\n",              bool_str[is_check(g, WHITE)]);
    }

    double score = heuristic(g, 1);
    if (score == -INFINITY)
        score = -999;
    else if (score == INFINITY)
        score = 999;
    printf("Estimated score: %lf\n", score);
    printf("Turns with no capture: %d\n", g->turns_without_captures);
}


static  void sigint_handler(int signal)
{
    paint_board(&sigint_state_copy, -1, -1);
    print_debug(&sigint_state_copy);
    dump_game_state(&sigint_state_copy);
    exit(0);
}

static double alpha_beta(struct game_state* g, double alpha, double beta, int depth)
{
    if (checkmate(g)) {
        return CHECKMATE_SCORE * (depth+1);
    }
    if (depth == 0) {
        return heuristic(g, depth) * g->player;
    }

    double m = alpha;

    // TODO: avoid doubly nested for loop
    for (int i=0; i<BOARD_SIZE; i++) {
        if (!friends(g->board[i], g->player))
            continue;

        for (int j=0; j<BOARD_SIZE; j++) {
            if (!move_ok(g, i, j))
                continue;

            typeof(*g) restore = *g;
            move(g, i, j);
            double x = -alpha_beta(g, -beta, -(alpha > m ? alpha : m), depth-1);
            *g = restore;
            m = m > x ? m : x;
            if (m >= beta) {
                return m;
            }
        }
    }

    return m;
}

static void computer_move(struct game_state* g, int depth, index_t* from, index_t* to)
{
    double m = -INFINITY;
    *from = -1;
    *to = -1;

    // TODO: avoid doubly nested for loop
    for (int i=0; i<BOARD_SIZE; i++) {
        if (!friends(g->board[i], g->player))
            continue;
        for (int j=0; j<BOARD_SIZE; j++) {
            if (!move_ok(g, i, j))
                continue;
            
            typeof(*g) restore = *g;
            move(g, i, j);
            if (checkmate(g)) {
                *g = restore;
                *to   = j;
                *from = i;
                return;
            }
            double x = -alpha_beta(g, -INFINITY, -m, depth-1);
            *g = restore;

            if (x > m) {
                //printf("considering %s to %s with score %lf\n", tile_str[i], tile_str[j], x);
                m     = x;
                *to   = j;
                *from = i;
            }
        }
    }
    assert(m != -INFINITY);
}

int main()
{
    if(signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Unable to catch SIGINT");
        exit(EXIT_FAILURE);
    }

    setlocale(LC_ALL, "C.UTF-8");

    struct game_state state = {};

    game_init(&state);

#if 0
    paint_board(&state);
    print_debug(&state, WHITE);
    printf("white threatmap:\n");
    print_threatmap(threatmap(&state, WHITE));
    printf("black threatmap:\n");
    print_threatmap(threatmap(&state, BLACK));
}
#else
    bool player_intervention = false;
    //double sum               = debug_sum_pieces(&state);
    index_t from = -1, to = -1;

    while (true) {
        printf("============================\n");
        paint_board(&state, from, to);
        printf("est. score: %lf", heuristic(&state, 0));
        //print_debug(&state);
        //dump_game_state(&state);

        if (player_intervention) {
            intervene:
            while (player_move(&state) == 0) {
                printf("Valid moves for %s:\n", state.player == WHITE ? "white" : "black");
            }
        } else {
            printf("%s to move, thinking...\n", state.player == WHITE ? "White" : "Black");
            computer_move(&state, MAX_DEPTH, &from, &to);
            if (from == -1 || to == -1) {
                printf("computer couldn't think, starting player intervention\n");
                player_intervention = true;
                goto intervene;
            }
            assert(move_ok(&state, from, to));
            move(&state, from, to);
            printf("Did %s to %s\n", tile_str[from], tile_str[to]);
        }

        sigint_state_copy = state;

        bool white_king = false;
        bool black_king = false;
        for (index_t i=0; i<BOARD_SIZE; i++) {
            if (state.board[i] == KING)
                white_king = true;
            if (state.board[i] == -KING)
                black_king = true;
        }
        assert(white_king);
        assert(black_king);

        if (is_check(&state, state.player)) {
            printf("\n%s is in check!\n", state.player == WHITE ? "White" : "Black");
        }
        if (checkmate(&state)) {
            printf("\nCheckmate. %s won!\n", state.player == WHITE ? "Black" : "White");
            //print_debug(&state, player);
            paint_board(&state, from, to);
            print_threatmap(threatmap(&state, -state.player));
            print_threatmap(valid_moves(&state, state.attr[attr_index(state.player)] & KING_POSITION));
            raise(SIGINT);
            break;
        }
        if (draw(&state)) {
            printf("\nDraw!\n");
            //print_debug(&state, player);
            paint_board(&state, from, to);
            break;
        }
    }

    return EXIT_SUCCESS;
}
#endif
