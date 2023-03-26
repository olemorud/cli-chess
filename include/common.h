
#include <stdbool.h> /* true, false, bool */
#include <stddef.h>  /* ptrdiff_t */
#include <stdint.h>  /* int32_t */

/** Type representing piece/tile on a chessboard */
typedef int32_t tile_t;

/** Type representing index of a chessboard tile */
typedef ptrdiff_t index_t;

#define BOARD_SIZE ((index_t)(8 * 8))

#define WHITE ((tile_t)1)
#define BLACK ((tile_t)-1)

#define E ((tile_t)0) ///< empty tile
#define K ((tile_t)1) ///< king
#define Q ((tile_t)2) ///< queen
#define R ((tile_t)3) ///< rook
#define B ((tile_t)4) ///< bishop
#define N ((tile_t)5) ///< knight
#define P ((tile_t)6) ///< pawn

#define ROW ((index_t)8)
#define COL ((index_t)1)
