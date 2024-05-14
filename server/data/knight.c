#include "../include/chess.h"

inherit LIB_PIECE;

static int valid_move(int dx, int dy)
{
	if (dx * dy != 2 && dx * dy != -2)
		return 0;

	return 1;
}
