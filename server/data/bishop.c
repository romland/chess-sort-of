#include "../include/chess.h"

inherit LIB_PIECE;

static int valid_move(int dx, int dy)
{
	/* Avoid division by 0 */
	if (!dx || !dy)
		return 0;

	if (dx != dy  &&  dx != -dy)
		return 0;

	return ::has_clear_path(dx, dy);
}
