
#include "common.h"
#include "util.h"


static bool bishop_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
static bool cardinal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
static bool diagonal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
static bool king_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
static bool knight_move_ok(index_t from, index_t to);
static bool pawn_move_ok(const tile_t board[BOARD_SIZE],
                   index_t      from,
                   index_t      to,
                   int          player);
static bool queen_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
static bool rook_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);


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

    if (abs_pos(col_diff) != abs_pos(row_diff))
        return false;

    for (index_t p = from + step; p != to; p += step) {
        if (! tile_empty(board[p]))
            return false;
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

	/* cardinal moves means one direction has to be zero */
    if (row_diff > 0 && col_diff > 0)
		return false;

    index_t step = row_diff ?
		  ROW * row_diff / abs_pos(row_diff)
        : col_diff / abs_pos(col_diff);

    for (index_t p = from + step; p != to; p += step) {
        if (! tile_empty(board[p]))
            return false;
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
    const index_t c = abs_pos(column(to) - column(from));
    const index_t r = abs_pos(row(to - from));

    return (c == 1 && r == 2)
        || (c == 2 && r == 1);
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
             int    player)
{
    /* player must own piece it moves */
    if (board[from] * player < 0)
        return false;

    if (tile_empty(board[from]))
        return false;

    /* player can't take own pieces or move piece onto itself*/
    if (same_color(board[from], board[to]))
        return false;

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
