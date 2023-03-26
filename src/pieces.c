
#include "common.h"
#include "util.h"

bool bishop_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool cardinal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool diagonal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool king_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool knight_move_ok(index_t from, index_t to);
bool pawn_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool queen_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);
bool rook_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to);

/**
 * Check if a move is valid
 *
 * \param board  Pointer to list of tiles representing board state
 * \param from   Tile to move piece from
 * \param to     Tile to move piece to
 * \param player The current player to move
 *
 * \return true if move is valid, false otherwise
 * */
bool move_ok(const tile_t board[BOARD_SIZE],
             index_t      from,
             index_t      to,
             int          player)
{
    /* player must own piece it moves */
    if (board[from] * player < 0)
        return false;

    /* empty tiles can't be moved */
    if (tile_empty(board[from]))
        return false;

    /* player can't take their own pieces or move a piece onto itself */
    if (same_color(board[from], board[to]))
        return false;

    /* check piece specific moves */
    switch (abs_tile(board[from])) {
    case P:
        return pawn_move_ok(board, from, to);

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

/**
 * Check if move is a valid pawn move
 *
 * \param board  Array of tiles representing chess board state
 * \param from   Index of board piece starts at
 * \param to     Index of board piece wants to move to
 *
 * \return true if move is valid, false otherwise
 */
bool pawn_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    const index_t direction = -1 * get_color(board[from]),
                  diff      = (to - from) * direction;

    switch (diff) {
    case ROW: /* single move */
        return tile_empty(board[to]);

    case ROW - COL: /* diagonal attack */
    case ROW + COL:
        return opposite_color(board[to], board[from]);

    case 2 * ROW: /* double move */
        return tile_empty(board[to])
            && tile_empty(board[from + ROW * direction])
            && (row(from) == 1 || row(from) == 6);

    default: /* any other move is illegal */
        return false;
    }
}

/**
 * Check if `to` is on a diagonal line of `from`
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 *
 * \return true if \p to is on a diagonal line of \p from
 */
bool diagonal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    // clang-format off
    const index_t col_diff = column(to) - column(from),
                  row_diff = row(to) - row(from),
				  x = get_sign(col_diff),
                  y = get_sign(row_diff) * ROW,
				  step = x + y;
    // clang-format on

    if (abs_index(col_diff) != abs_index(row_diff))
        return false;

    for (index_t p = from + step; p != to; p += step)
        if (! tile_empty(board[p]))
            return false;

    return true;
}

/**
 * Check if index `to` is on a cardinal line of `from`
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 *
 * \return true if \p to and \p from share a column or row
 */
bool cardinal_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    const index_t col_diff = column(to) - column(from),
                  row_diff = row(to - from);

    if (row_diff > 0 && col_diff > 0)
        return false;

    const index_t step =
        row_diff ? ROW * get_sign(row_diff) : get_sign(col_diff);

    for (index_t p = from + step; p != to; p += step)
        if (! tile_empty(board[p]))
            return false;

    return true;
}

/**
 * Check if move is a valid bishop move
 *
 * \param board Array of tiles representing chess board state
 * \param from  Index of board piece is at
 * \param to    Index of board piece tries to move to
 *
 * \return true if move is valid, false otherwise
 */
bool bishop_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    return diagonal_move_ok(board, from, to);
}

/**
 * Check if move is a valid rook move
 *
 * \param board Array of tiles representing chess board state
 * \param from  Index of board piece is at
 * \param to    Index of board piece tries to move to
 *
 * \return true if move is valid, false otherwise
 */
bool rook_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    return cardinal_move_ok(board, from, to);
}

/**
 * Check if move is a valid knight move
 *
 * \param from  Index of board piece is at
 * \param to    Index of board piece tries to move to
 *
 * \return true if move is valid, false otherwise
 */
bool knight_move_ok(index_t from, index_t to)
{
    const index_t x = abs_index(column(to) - column(from)),
                  y = abs_index(row(to - from));

    return (x == 1 && y == 2) || (x == 2 && y == 1);
}

/**
 * Check if move is a valid king move
 *
 *  \param board Array of tiles representing chess board state
 *  \param from  Index of board piece is at
 *  \param to    Index of board piece tries to move to
 *
 * \return true if move is valid, false otherwise
 */
bool king_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    const index_t abs_x = abs_index(column(to) - column(from)),
                  abs_y = abs_index(row(to) - row(from));

    // TODO: check if move causes check
    (void)board;

    return abs_x <= 1 && abs_y <= 1;
}

/**
 * Check if move is a valid queen move
 *
 * \param board Array of tiles representing chess board state
 * \param from  Index of board piece is at
 * \param to    Index of board piece tries to move to
 *
 * \return true if move is valid, false otherwise
 */
bool queen_move_ok(const tile_t board[BOARD_SIZE], index_t from, index_t to)
{
    return diagonal_move_ok(board, from, to)
        || cardinal_move_ok(board, from, to);
}
