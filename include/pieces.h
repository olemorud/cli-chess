
#ifndef PIECES_H
#define PIECES_H

#include "common.h"

bool move_ok(const tile_t board[BOARD_SIZE],
             index_t      from,
             index_t      to,
             int          player);

#endif
