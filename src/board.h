
#pragma once

#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef uint64_t bitmap_t;
typedef uint64_t index_t;
typedef uint64_t piece_t;

#define TILE_BITS 4ULL

enum piece : piece_t {
    EMPTY                = 0b0000ULL,

    W_KING               = 0b0001ULL,
    W_QUEEN              = 0b0010ULL,
    W_ROOK               = 0b0011ULL,
    W_BISHOP             = 0b0100ULL,
    W_KNIGHT             = 0b0101ULL,
    W_PAWN               = 0b0110ULL,
    W_PAWN_ENPASSENTABLE = 0b0111ULL,

    B_EMPTY              = 0b1000ULL,
    B_KING               = 0b1001ULL,
    B_QUEEN              = 0b1010ULL,
    B_ROOK               = 0b1011ULL,
    B_BISHOP             = 0b1100ULL,
    B_KNIGHT             = 0b1101ULL,
    B_PAWN               = 0b1110ULL,
    B_PAWN_ENPASSENTABLE = 0b1111ULL,

    BLACK                = 0b1000ULL,
    #define WHITE 0
};

static const char * const piece_str[] = {
    [EMPTY]                 = "EMPTY",
    [W_KING]                = "W_KING",
    [W_QUEEN]               = "W_QUEEN",
    [W_ROOK]                = "W_ROOK",
    [W_BISHOP]              = "W_BISHOP",
    [W_KNIGHT]              = "W_KNIGHT",
    [W_PAWN]                = "W_PAWN",
    [W_PAWN_ENPASSENTABLE]  = "W_PAWN (enpassentable)",

    [B_EMPTY]               = "EMPTY (BLACK flag set)",
    [B_KING]                = "B_KING",
    [B_QUEEN]               = "B_QUEEN",
    [B_ROOK]                = "B_ROOK",
    [B_BISHOP]              = "B_BISHOP",
    [B_KNIGHT]              = "B_KNIGHT",
    [B_PAWN]                = "B_PAWN",
    [B_PAWN_ENPASSENTABLE ] = "B_PAWN (enpassentable)",
};

static const wchar_t piece_symbol[] = {
    [WHITE | 0]            = ' ',
    [W_KING]               = 0x265A,
    [W_QUEEN]              = 0x265B,
    [W_ROOK]               = 0x265C,
    [W_BISHOP]             = 0x265D,
    [W_KNIGHT]             = 0x265E,
    [W_PAWN]               = 0x265F,
    [W_PAWN_ENPASSENTABLE] = 0x265F,
    [B_KING]               = 0x265A,
    [B_QUEEN]              = 0x265B,
    [B_ROOK]               = 0x265C,
    [B_BISHOP]             = 0x265D,
    [B_KNIGHT]             = 0x265E,
    [B_PAWN]               = 0x265F,
    [B_PAWN_ENPASSENTABLE] = 0x265F,
};

struct board {
    bitmap_t pieces[4]; // 4 bits correspond to 1 tile
};
static_assert(sizeof(((struct board*)0)->pieces) * CHAR_BIT == TILE_BITS*64,
       "pieces must contain enough information to hold a 64 chessboard tiles");
static_assert((sizeof(((struct board*)0)->pieces[0]) * CHAR_BIT) % TILE_BITS == 0,
       "bitmap_t in bits must be divisible by TILE_BITS");

static inline bitmap_t tile_mask(index_t i)
{
    return 0b1111ULL << (TILE_BITS*i);
}

static inline bool is_white(piece_t piece)
{
    return (piece & BLACK) == 0;
}

static inline bool is_black(piece_t piece)
{
    return (piece & BLACK) == BLACK;
}

static inline piece_t piece_at(struct board* board, index_t tile)
{
    static_assert(sizeof board->pieces[0] * CHAR_BIT == 64, "bad refactor");
    return (tile_mask(tile % (64/TILE_BITS)) & board->pieces[tile / (64/TILE_BITS)]) >> 4ULL*tile;
}

static void paint_board(struct board* b)
{
    /* https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
       The unicode symbols for the pieces are calculated from adding
       0x2653 (#define'd as UNICODE_CHESS_SYMBOL) with the piece value. */
#define BG_RED()       printf("\033[48;2;150;150;0m")
#define BG_DARKBLUE()  printf("\033[48;2;100;100;150m")
#define BG_LIGHTBLUE() printf("\033[48;2;150;150;200m")
#define FG_BLACK()     printf("\033[38;2;0;0;0m")
#define FG_WHITE()     printf("\033[38;2;255;255;255m")
    for (int i = 7; i >= 0; i--) {
        printf("\n %d ", i+1); // number coordinates
        for (int j = 0; j < 8; j++) {
            piece_t p = piece_at(b, i*8+j);
            if ((i + j) % 2) {
                BG_DARKBLUE();
            } else {
                BG_LIGHTBLUE();
            }

            if (p == EMPTY) {
                printf("  ");
                continue;
            }
            if (is_white(p)) {
                FG_WHITE();
            } else {
                FG_BLACK();
            }
            printf("%lc ", piece_symbol[p]);
        }
        printf("\033[0m"); // reset text attributes
    }
    /* horizontal letter coordinates */
    printf("\n  ");
    for (int i = 0; i < 8; i++)
        printf(" %c", 'a' + i);
    printf("\n");
}
