#include "../include/chess.h"

inherit LIB_PIECE;

static int valid_move(int dx, int dy)
{
	if (dx < -1 || 1 < dx || dy < -1 || 1 < dy)
		return 0;

	return 1;
}


int bored()
{
	return 0;
}
