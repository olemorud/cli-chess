
#include "util.h"

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
 * Returns true if a and b are tiles of opposite color, false otherwise
 *
 * \param a Tile to compare
 * \param b Tile to compare it with
 * */
bool opposite_color(tile_t a, tile_t b)
{
    return a * b < 0;
}

/**
 * Returns true if a and b are pieces of the same color, false otherwise
 *
 * \param a Tile to compare
 * \param b Tile to compare it with
 * */
bool same_color(tile_t a, tile_t b)
{
    return a * b > 0;
}


