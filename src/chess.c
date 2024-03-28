
#include <assert.h>
#include <ctype.h>   /* isalpha, isdigit ... */
#include <locale.h>  /* setlocale */
#include <math.h>
#include <stdbool.h> /* true, false, bool */
#include <stddef.h>  /* ptrdiff_t */
#include <stdint.h>  /* int32_t */
#include <stdio.h>   /* printf, scanf */
#include <string.h>  /* memcpy */
#include <stdlib.h>

typedef int32_t   piece_t;
typedef ptrdiff_t index_t;
typedef uint64_t  bitmap_t;

#define MAX_DEPTH 5 // TODO: don't hardcode this
#define N_ELEMS(x) (sizeof x / sizeof x[0])
#define BOARD_SIZE ((index_t)64)
#define BIT(x) (1UL << (x))

#define WHITE  ((piece_t)1)
#define BLACK  ((piece_t)-1)

#define EMPTY  ((piece_t)0)
#define KING   ((piece_t)1)
#define QUEEN  ((piece_t)2)
#define ROOK   ((piece_t)3)
#define BISHOP ((piece_t)4)
#define KNIGHT ((piece_t)5)
#define PAWN   ((piece_t)6)

#define ROW ((index_t)8)
#define COL ((index_t)1)

struct game_state {
    index_t king_pos[2];
    bool    a_rook_touched[2];
    bool    h_rook_touched[2];
    bool    king_touched[2];

    int turns_without_captures;

    piece_t board[BOARD_SIZE];
};

enum castle_type {
    CASTLE_KINGSIDE  = 1,
    CASTLE_QUEENSIDE = 2,
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

static const double piece_position_bonus[7][BOARD_SIZE * sizeof(double)] = {
    [EMPTY] = {0},
    [PAWN] = {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.0, 
        1.0, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.0, 
        1.2, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.2, 
        1.7, 1.0, 1.4, 1.4, 1.4, 1.4, 1.0, 1.7, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
    [BISHOP] = {
        1.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.2, 
        1.2, 1.2, 1.0, 1.0, 1.0, 1.2, 1.2, 1.0, 
        1.0, 1.2, 1.2, 1.0, 1.2, 1.2, 1.0, 1.0, 
        1.0, 1.0, 1.2, 1.2, 1.2, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.2, 1.2, 1.2, 1.0, 1.0, 1.0, 
        1.0, 1.2, 1.2, 1.0, 1.2, 1.2, 1.0, 1.0, 
        1.2, 1.2, 1.0, 1.0, 1.0, 1.2, 1.2, 1.0, 
        1.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.2, 
    },
    [KNIGHT] = {
        0.5, 0.7, 0.8, 0.8, 0.8, 0.8, 0.7, 0.5, 
        0.6, 0.7, 0.9, 0.9, 0.9, 0.9, 0.7, 0.6, 
        0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
        0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
        0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
        0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.6, 
        0.6, 0.7, 0.9, 0.9, 0.9, 0.9, 0.7, 0.6, 
        0.5, 0.7, 0.8, 0.8, 0.8, 0.8, 0.7, 0.5, 
    },
    [ROOK] = {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
    [QUEEN] = {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
    [KING] = {
       /*a    b    c    d    e    f    g    h*/
        1.4, 1.4, 1.4, 1.0, 1.0, 1.0, 99,  1.6, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    },
};


#define TILE(col, row) ((col)-'A'+ ROW*((row)-1))

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

static inline index_t row(index_t i)
{
    return i / ROW;
}

static inline index_t column(index_t i)
{
    return i % ROW;
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
        [PAWN]   = "PAWN",
        [BISHOP] = "BISHOP",
        [KNIGHT] = "KNIGHT",
        [ROOK]   = "ROOK",
        [QUEEN]  = "QUEEN",
        [KING]   = "KING",
    };
    p = piece_abs(p);
    if (p >= 0 && (size_t)p < sizeof table / sizeof *table) {
        return table[piece_abs(p)];
    } else {
        return "UNKNOWN";
    }
}

static void board_paint(struct game_state* g)
{
    /* https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
       The unicode symbols for the pieces are calculated from adding
       0x2653 (#define'd as UNICODE_CHESS_SYMBOL) with the piece value. */
#define BG_DARKBLUE()  printf("\033[48;2;100;100;150m")
#define BG_LIGHTBLUE() printf("\033[48;2;150;150;200m")
#define FG_BLACK()     printf("\033[38;2;0;0;0m")
#define FG_WHITE()     printf("\033[38;2;255;255;255m")
#define UNICODE_CHESS_SYMBOL 0x2659
    for (ssize_t i = 7; i >= 0; i--) {
    //for (ssize_t i = 0; i < 8; i++) {
        printf("\n %zu ", i+1); // number coordinates
        for (size_t j = 0; j < 8; j++) {
            piece_t t = g->board[i * 8 + j];
            if ((i + j) % 2)
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
}

static void move(struct game_state* g, index_t from, index_t to)
{
    int p = (piece_color(g->board[from]) == WHITE ? 0 : 1);

    if (piece_abs(g->board[from]) == KING) {
        g->king_pos[p] = to;
        g->king_touched[p] = true;
    } else if (from == TILE('A', 8)) {
        g->a_rook_touched[0] = true;
    } else if (from == TILE('A', 1)) {
        g->a_rook_touched[1] = true;
    } else if (from == TILE('H', 1)) {
        g->h_rook_touched[0] = true;
    } else if (from == TILE('H', 8)) {
        g->h_rook_touched[1] = true;
    }

    if (g->board[to] == EMPTY) {
        g->turns_without_captures += 1;
    } else {
        g->turns_without_captures = 0;
    }

    if (piece_abs(g->board[from]) == KING) {
        switch (to) {
        case TILE('G', 1):
            g->board[TILE('E', 1)] = EMPTY;
            g->board[TILE('F', 1)] = ROOK;
            g->board[TILE('G', 1)] = KING;
            g->board[TILE('H', 1)] = EMPTY;
            break;
        case TILE('C', 1):
            g->board[TILE('A', 1)] = EMPTY;
            g->board[TILE('B', 1)] = EMPTY;
            g->board[TILE('C', 1)] = KING;
            g->board[TILE('D', 1)] = ROOK;
            g->board[TILE('E', 1)] = EMPTY;
            break;
        case TILE('G', 8):
            g->board[TILE('E', 8)] = EMPTY;
            g->board[TILE('F', 8)] = ROOK;
            g->board[TILE('G', 8)] = KING;
            g->board[TILE('H', 8)] = EMPTY;
            break;
        case TILE('C', 8):
            g->board[TILE('A', 8)] = EMPTY;
            g->board[TILE('B', 8)] = EMPTY;
            g->board[TILE('C', 8)] = KING;
            g->board[TILE('D', 8)] = ROOK;
            g->board[TILE('E', 8)] = EMPTY;
            break;
        default:
            g->board[to]   = g->board[from];
            g->board[from] = EMPTY;
            return;
        }
    } else {
        g->board[to]   = g->board[from];
        g->board[from] = EMPTY;
    }
}

static bitmap_t pawn_threatmap(struct game_state* g, index_t index)
{
    const index_t direction = piece_color(g->board[index]);
    const index_t left  = BIT(index + ROW*direction - 1);
    const index_t right = BIT(index + ROW*direction + 1);

    if (column(index) == column(TILE('A', 1)))
        return right;

    if (column(index) == column(TILE('H', 1)))
        return left;

    return left | right;
}

static bitmap_t diagonal_threatmap(struct game_state* g, index_t index)
{
    bitmap_t threatened = 0;

    //index_t directions[] = { ROW+1, ROW-1, -ROW+1, -ROW-1 };

    for (index_t i = index+ROW+1; i < BOARD_SIZE && column(i-1) != 7; i += ROW+1) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    for (index_t i = index+ROW-1; i < BOARD_SIZE && column(i+1) != 0; i += ROW-1) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    for (index_t i = index-ROW+1; i >= 0 && column(i-1) != 7; i += -ROW+1) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    for (index_t i = index-ROW-1; i >= 0 && column(i+1) != 0; i += -ROW-1) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY) {
            break;
        }
    }
    return threatened;
}

static bitmap_t cardinal_threatmap(struct game_state* g, index_t index)
{
    bitmap_t threatened = 0;

    for (index_t i = index+ROW; i < BOARD_SIZE; i += ROW) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY)
            break;
    }
    for (index_t i = index-ROW; i >= 0; i -= ROW) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY)
            break;
    }
    for (index_t i = index+1; i < BOARD_SIZE && column(i) != 0; i++) {
        threatened |= BIT(i);
        if (g->board[i] != EMPTY)
            break;
    }
    for (index_t i = index-1; i > 0 && column(i) != 7; i--) {
        threatened |= BIT(i);
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
         1,   2,
         1,  -2,
        -1,   2,
        -1,  -2,
         2,   1,
         2,  -1,
        -2,   1,
        -2,  -1
    };
    // clang-format on
    
    for (size_t i = 0; i < N_ELEMS(knight_wheel); i += 2) {
        index_t atks = index +  knight_wheel[i] + knight_wheel[i+1] * ROW;

        if (column(index) + knight_wheel[i] < 0
         || column(index) + knight_wheel[i] > 7
         || row(index) + knight_wheel[i+1] < 0
         || row(index) + knight_wheel[i+1] > 7)
            continue;

        threatened |= BIT(atks);
    }

    return threatened;
}

static bitmap_t king_threatmap(index_t index)
{
    // I fucking hate this function so much
    if (row(index) == 0) {
        if (column(index) == 0) {
            return BIT(index+1) | BIT(index+ROW+1) | BIT (index+ROW);
        }
        else if (column(index) == 7) {
            return BIT(index-1) | BIT(index+ROW-1) | BIT (index+ROW);
        }
        else {
            return BIT(index-1) | BIT(index+1) | BIT(index+ROW-1) | BIT(index+ROW) | BIT(index+ROW+1);
        }
    }
    if (row(index) == 7) {
        if (column(index) == 0) {
            return BIT(index+1) | BIT(index-ROW+1) | BIT (index-ROW);
        }
        else if (column(index) == 7) {
            return BIT(index-1) | BIT(index-ROW-1) | BIT (index-ROW);
        }
        else {
            return BIT(index-1) | BIT(index+1) | BIT(index-ROW-1) | BIT(index-ROW) | BIT(index-ROW+1);
        }
    } else {
        if (column(index) == 0) {
            return BIT(index-ROW) | BIT(index-ROW+1) | BIT(index+1) | BIT(index+ROW+1) | BIT(index+ROW);
        }
        else if (column(index) == 7) {
            return BIT(index-ROW) | BIT(index-ROW-1) | BIT(index-1) | BIT(index+ROW-1) | BIT(index+ROW);
        } else {
            return BIT(index-ROW-1) | BIT(index-ROW) | BIT(index-ROW+1)
                 | BIT(index-1)                      | BIT(index+1)
                 | BIT(index+ROW-1) | BIT(index+ROW) | BIT(index+ROW+1);
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
        fputc('\n', stdout);
        for (size_t j = 0; j < 8; j++) {
            fputc(' ', stdout);
            if (threatmap & BIT(i*ROW+j))
                fputc('x', stdout);
            else
                fputc('-', stdout);
        }
    }
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

static bitmap_t threatmap(struct game_state* g, int attacker)
{
    bitmap_t threatened = 0;
    for(index_t i = 0; i < BOARD_SIZE; i++) {
        if (friends(g->board[i], attacker)) {
            threatened |= piece_threatmap(g, i);
        }
    }

    return threatened;
}

static bool pawn_move_ok(struct game_state* g, index_t from, index_t to)
{
    const index_t direction = piece_color(g->board[from]);
    const index_t diff      = (to - from) * direction;

    //printf("\n");

    switch (diff) {
    case ROW: /* single move */
        //if (!(g->board[to] == EMPTY)) {
        //    printf("pawn must move to empty tile, ");
        //}
        return g->board[to] == EMPTY;

    case ROW - COL: /* diagonal attack */
    case ROW + COL:
        //if (!(enemies(g->board[to], g->board[from]))) {
        //    printf("pawn can't take its own pieces, ");
        //}
        return enemies(g->board[to], g->board[from]);

    case 2 * ROW: /* double move */
        //if (!( g->board[to] == EMPTY)) {
        //    printf("pawn must move to empty piece, ");
        //}
        //if (!( g->board[from + ROW * direction] == EMPTY)) {
        //    printf("pawn can't jump over piece, ");
        //}
        //if (!( (row(from) == 1 || row(from) == 6))) {
        //    printf("pawn can only move two steps from its starting row, ");
        //}
        //printf("\n");
        return g->board[to] == EMPTY
            && g->board[from + ROW * direction] == EMPTY
            && (row(from) == 1 || row(from) == 6);

    default: /* any other move is illegal */
        //printf("pawn move is generally illegal, ");
        return false;
    }
}

static bool king_move_ok(struct game_state* g, index_t from, index_t to)
{
    // castling check is implemented as a separate function
    return BIT(to) & king_threatmap(from)
        && BIT(to) & ~threatmap(g, -piece_color(g->board[from]));
}

bool is_check(struct game_state* g, int player)
{
    return BIT(g->king_pos[player == WHITE ? 0 : 1]) & threatmap(g, -player);
}

static bool castle_ok(struct game_state* g, int player, enum castle_type t)
{
    if (is_check(g, player)) {
        return false;
    }
    const int p = player == WHITE ? 0 : 1;
    const int row = (player == WHITE ? 1 : 8);

    if (t == CASTLE_KINGSIDE) {
        return g->h_rook_touched[p] == false
            && g->king_touched[p] == false
            && ~threatmap(g, -player) & (BIT(TILE('F', row)) | BIT(TILE('G', row)))
            && g->board[TILE('G', row)] == EMPTY
            && g->board[TILE('F', row)] == EMPTY;
    } else if (t == CASTLE_QUEENSIDE) {
        return g->a_rook_touched[p] == false
            && g->king_touched[p] == false
            && ~threatmap(g, -player) & (BIT(TILE('C', row)) | BIT(TILE('D', row)))
            && g->board[TILE('B', row)] == EMPTY
            && g->board[TILE('C', row)] == EMPTY
            && g->board[TILE('D', row)] == EMPTY;
    }

    fprintf(stderr, "invalid castle_type passed to %s", __func__);
    exit(EXIT_FAILURE);
}


bool move_ok(struct game_state* g, index_t from, index_t to, int player)
{
    /* Player must own piece it moves
       and a player can't capture their own pieces. */
    if (enemies(player, g->board[from]) || friends(player, g->board[to])) {
        return false;
    }

    bool move_ok = false;;

    switch (piece_abs(g->board[from])) {
    case EMPTY:
        move_ok = false;
        break;
    case PAWN:
        move_ok = pawn_move_ok(g, from, to);
        break;
    case KING:
        if (to == TILE('G', 8) || to == TILE('G', 1)) {
            move_ok = castle_ok(g, player, CASTLE_KINGSIDE);
        } else if (to == TILE('C', 8) || to == TILE('C', 1)) {
            move_ok = castle_ok(g, player, CASTLE_QUEENSIDE);
        } else {
            move_ok = king_move_ok(g, from, to);
        }
        break;
    default:
        move_ok = BIT(to) & piece_threatmap(g, from);
        break;
    }

    if (!move_ok) {
        return false;
    }

    typeof(*g) restore = *g;
    move(g, from, to);
    bool bad = is_check(g, player);
    *g = restore;
    //memcpy(g, &restore, sizeof restore);

    if (bad) {
        return false;
    }

    return true;
}

static bitmap_t valid_moves(struct game_state* g, index_t i, int player)
{
    bitmap_t output = 0;
    for (index_t j=0; j<BOARD_SIZE; j++) {
        if (move_ok(g, i, j, player)) {
            output |= BIT(j);
        }
    }
    return output;
}

static inline bool draw(struct game_state* g, int player)
{
    (void)player;
    // TODO: implement stalemate
    return g->turns_without_captures >= 50;
}

// TODO: fix this garbage
static bool checkmate(struct game_state* g, int player)
{
    if (!is_check(g, player))
        return false;

    // TODO: avoid doubly nested for loop
    for (int i=0; i<BOARD_SIZE; i++) {
        for (int j=0; j<BOARD_SIZE; j++) {
            if (move_ok(g, i, j, player)) {
                return false;
            }
        }
    }

    return true;
}

static void game_init(struct game_state* g)
{
    // black pieces are prefixed by a minus (-)
    // clang-format off
#if 1
    static const piece_t start[BOARD_SIZE] = {
            ROOK,   KNIGHT, BISHOP, QUEEN,  KING,   BISHOP, KNIGHT, ROOK,
            PAWN,   PAWN,   PAWN,   PAWN,   PAWN,   PAWN,   PAWN,   PAWN,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
           -PAWN,  -PAWN,  -PAWN,  -PAWN,  -PAWN,  -PAWN,  -PAWN,  -PAWN,
           -ROOK,  -KNIGHT,-BISHOP,-QUEEN, -KING,  -BISHOP,-KNIGHT,-ROOK,
    };
    // clang-format on  
#else
    static const piece_t start[BOARD_SIZE] = {
            ROOK,   EMPTY,  EMPTY,  EMPTY,  KING,   EMPTY,  EMPTY,  ROOK,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,  EMPTY,
            EMPTY,  EMPTY,  EMPTY,  EMPTY,  -KING,  EMPTY,  EMPTY,  EMPTY,
    };
#endif

    memset(g, 0, sizeof *g);
    memcpy(g->board, start, sizeof start);

    g->king_pos[0] = TILE('E', 1);
    g->king_pos[1] = TILE('E', 8);
}

static index_t input_to_index(char input[2])
{
    const char col = toupper(input[0]);
    const char row = input[1];

    if (col < 'A' || col > 'H' || row < '1' || row > '8')
        return -1;

    return TILE(col, row - '0');
}

// TODO: Implement algebaric notation
static int player_move(int turn_no, struct game_state* g)
{
    char input[3] = { 0 };

    int from = -1, to = -1;

    printf("\n%s's turn to move", turn_no % 2 ? "Black" : "White");
    printf("\nMove piece\nfrom: ");
    scanf(" %2s", input);

    from = input_to_index(input);

    if (from == -1)
        return 0;

    print_threatmap(valid_moves(g, from, turn_no % 2 ? BLACK : WHITE));
    printf("\nto: ");
    scanf(" %2s", input);

    to = input_to_index(input);

    if (to == -1)
        return 0;

    if (!move_ok(g, from, to, turn_no % 2 ? BLACK : WHITE))
        return 0;

    move(g, from, to);

    return 1;
}

static double heuristic(struct game_state* g, int player)
{
    if (draw(g, player))
        return 0;

    if (checkmate(g, -player))
        return player * INFINITY;

    double score = 0;
    for (index_t i=0; i<BOARD_SIZE; i++) {
        const piece_t piece = g->board[i];
        const int     player = piece_color(piece);
        const piece_t type = piece_abs(piece);
        if (player == WHITE) {
            score += player * piece_value[type] * piece_position_bonus[type][i];
        } else {
            score += player * piece_value[type] * piece_position_bonus[type][BOARD_SIZE-i-1];
        }
    }
    return score;
}

static void print_debug(struct game_state* g, int player)
{
    //struct game_state {
    //  piece_t board[BOARD_SIZE];
    //  index_t king_pos[2];
    //  bool    a_rook_touched[2];
    //  bool    h_rook_touched[2];
    //  bool    king_touched[2];
    //};
    static const char * const bool_str[2] = {"False", "True"};

    printf("White A king touched: %s\n", bool_str[g->king_touched[0]]);
    printf("White A rook touched: %s\n", bool_str[g->a_rook_touched[0]]);
    printf("White H rook touched: %s\n", bool_str[g->h_rook_touched[0]]);
    printf("White can king side castle: %s\n", bool_str[castle_ok(g, WHITE, CASTLE_KINGSIDE)]);
    printf("White can queen side castle: %s\n", bool_str[castle_ok(g, WHITE, CASTLE_QUEENSIDE)]);
    printf("White king pos: %c%ld\n", 'A'+((char)column(g->king_pos[0])), 1+row(g->king_pos[0]));
    
    //printf("Black A king touched: %s\n", bool_str[g->king_touched[1]]);
    //printf("Black A rook touched: %s\n", bool_str[g->a_rook_touched[1]]);
    //printf("Black H rook touched: %s\n", bool_str[g->h_rook_touched[1]]);
    //printf("Black can king side castle: %s\n", bool_str[castle_ok(g, BLACK, CASTLE_KINGSIDE)]);
    //printf("Black can queen side castle: %s\n", bool_str[castle_ok(g, BLACK, CASTLE_QUEENSIDE)]);
    //printf("Black king pos: %c%ld\n", 'A'+((char)column(g->king_pos[1])), 1+row(g->king_pos[1]));

    printf("Turns with no capture: %d\n", g->turns_without_captures);
    printf("Estimated score: %lf\n", heuristic(g, player));
}

static double alpha_beta(struct game_state* g, double alpha, double beta, int player, int depth)
{
    if (depth == 0)
        return heuristic(g, player) * player;

    double m = alpha;
    unsigned char restore[sizeof *g];

    // TODO: avoid doubly nested for loop
    for (int i=0; i<BOARD_SIZE; i++) {
        if (g->board[i] == EMPTY || !friends(g->board[i], player))
            continue;
        for (int j=0; j<BOARD_SIZE; j++) {
            if (!move_ok(g, i, j, player))
                continue;

            memcpy(restore, g, sizeof *g);
            move(g, i, j);
            double x = -alpha_beta(g, -beta, -m, -player, depth-1);
            memcpy(g, restore, sizeof *g);

            // alpha-beta pruning
            if (x > m) {
                m = x;
            }
            if (m >= beta) {
                return m;
            }
        }
    }

    return m;
}

static void computer_move(struct game_state* g, int player, int depth, index_t* from, index_t* to)
{
    unsigned char restore[sizeof *g];

    double max = -INFINITY;

    // TODO: avoid doubly nested for loop
    for (int i=0; i<BOARD_SIZE; i++) {
        if (!friends(g->board[i], player))
            continue;
        for (int j=0; j<BOARD_SIZE; j++) {
            if (!move_ok(g, i, j, player))
                continue;
            
            memcpy(restore, g, sizeof *g);
            move(g, i, j);
            double x = -alpha_beta(g, -INFINITY, -max, -player, depth-1);
            memcpy(g, restore, sizeof *g);

            if (x > max) {
                *to = j;
                *from = i;
                max = x;
            }
            //printf("beta: %lf, max: %lf\n", beta, max);
        }
    }
    //assert(max != -INFINITY);
}

int main()
{
    setlocale(LC_ALL, "C.UTF-8");

    struct game_state state = {};

    game_init(&state);

#if 0
    board_paint(&state);
    print_debug(&state, WHITE);
    printf("white threatmap:\n");
    print_threatmap(threatmap(&state, WHITE));
    printf("black threatmap:\n");
    print_threatmap(threatmap(&state, BLACK));
}
#else

    int turn = 0;
    int player = WHITE;
    bool player_intervention = false;

    while (true) {
        print_debug(&state, player);
        //print_threatmap(threatmap(&state, turn % 2 ? BLACK : WHITE));
        board_paint(&state);

        if (is_check(&state, player)) {
            printf("%s in check!\n", player == WHITE ? "White" : "Black");
            print_threatmap(piece_threatmap(&state, state.king_pos[player == WHITE ? 0 : 1]));
        }

        if (player_intervention) {
            intervene:
            while (player_move(turn, &state) == 0) {
                printf("Valid moves for %s:\n", player == WHITE ? "white" : "black");
                print_threatmap(valid_moves(&state, state.king_pos[player == WHITE ? 0 : 1], player));
            }
        } else {
            printf("thinking...\n");
            index_t from = -1, to = -1;
            computer_move(&state, player, MAX_DEPTH, &from, &to);
            if (from == -1 || to == -1) {
                printf("computer couldn't think, starting player intervention\n");
                printf("Valid moves for %s:\n", player == WHITE ? "white" : "black");
                print_threatmap(valid_moves(&state, state.king_pos[player == WHITE ? 0 : 1], player));
                printf("\nThreatened pieces by %s\n", -player == WHITE ? "white" : "black");
                print_threatmap(threatmap(&state, -player));
                player_intervention = true;
                goto intervene;
            }
            move(&state, from, to);
            printf("Did %c%ld to %c%ld\n", (char)('A'+column(from)), 1+row(from), (char)('A'+column(to)), 1+row(to));
        }

        turn += 1;
        player = -player;

        if (checkmate(&state, player)) {
            printf("\nCheckmate. %s won!\n", -player == WHITE ? "White" : "Black");
            print_debug(&state, player);
            board_paint(&state);
            print_threatmap(valid_moves(&state, state.king_pos[player == WHITE ? 0 : 1], player));
            print_threatmap(threatmap(&state, -player));
            break;
        }
        if (draw(&state, player)) {
            printf("\nDraw!\n");
            print_debug(&state, player);
            board_paint(&state);
            break;
        }
    }

    return EXIT_SUCCESS;
}
#endif
