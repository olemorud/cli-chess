
#include "util.h"

/**
 * Calculate the absolute value of an index_t value
 *
 * \param p positive or negative index_t
 *
 * \return the absolute value of p
 */
index_t abs_index(index_t i)
{
    if (i < 0)
        return -1 * i;

    return i;
}

/**
 * Calculate the absolute value of a tile_t value
 *
 * \param t positive or negative tile_t
 *
 * \return the absolute value of t
 * */
tile_t abs_tile(tile_t t)
{
    if (t < 0)
        return -1 * t;

    return t;
}

/**
 * Check if tile is has no piece on it
 *
 * \param t tile to check if empty
 *
 * \return true if tile is empty, false otherwise
 * */
bool tile_empty(tile_t t)
{
    return t == E;
}

/**
 * Get row of index
 *
 * \param i index to get row number of
 *
 * \return row number of i
 * */
index_t row(index_t i)
{
    return i / ROW;
}

/**
 * Get column of index
 *
 * \param i index to get column number of
 *
 * \return column number of i
 * */
index_t column(index_t i)
{
    return i % ROW;
}

/**
 * Check if two tiles have pieces of the opposite color
 *
 * \param a Tile to compare
 * \param b Tile to compare it with
 *
 * \return true if a and b are opposite colors, false otherwise
 * */
bool opposite_color(tile_t a, tile_t b)
{
    return a * b < 0;
}

/**
 * Check if two tiles have pieces of the same color
 *
 * \param a Tile to compare
 * \param b Tile to compare it with
 *
 * \return true if a and b are opposite colors, false otherwise
 * */
bool same_color(tile_t a, tile_t b)
{
    return a * b > 0;
}
