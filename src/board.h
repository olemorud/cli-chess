
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef uint64_t     bitmap_t;
typedef uint_fast8_t index_t;
typedef int_fast8_t  piece_t;

enum tile_info {
    EMPTY     = 0b0000,
    KING      = 0b0001,
    QUEEN     = 0b0010,
    ROOK      = 0b0011,
    BISHOP    = 0b0100,
    KNIGHT    = 0b0101,
    PAWN      = 0b0110,
    COLOR_BIT = 0b1000,

    PIECE_UNKNOWN=-1
};

struct board {
    static_assert(sizeof(bitmap_t) == 64/8, "bitmap_t must be 64 bits long");
    bitmap_t pieces[4]; // 4 bits correspond to 1 tile
};

static inline bitmap_t bit(index_t i)
{
    return 0b111UL << 3*i;
}

static bool is_white(piece_t piece)
{
    return piece % 2 == 0;
}

static bool is_black(piece_t piece)
{
    return piece % 2 == 1;
}

static piece_t piece_at(struct board* board, index_t tile)
{
    for (size_t i = 0; i < sizeof board->pieces / board->pieces[0]; i++) {
        if (bit(tile) & board->pieces[i]) {
            return i;
        }
    }
    return PIECE_UNKNOWN;
}


