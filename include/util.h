#include "common.h"

int64_t get_sign(int64_t n);
bool    tile_empty(tile_t t);
index_t abs_index(index_t i);
index_t column(index_t i);
index_t row(index_t i);
tile_t  abs_tile(tile_t t);
tile_t  get_color(tile_t t);
bool    same_color(tile_t a, tile_t b);
bool    opposite_color(tile_t a, tile_t b);
