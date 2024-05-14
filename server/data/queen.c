#include "../include/chess.h"

inherit LIB_PIECE;

static int valid_move(int dx, int dy)
{
	if (dx == 0 && dy == 0)
		return 0;

	if (dx != 0  &&  dy != 0  &&  dx != dy  &&  dx != -dy)
		return 0;

	return ::has_clear_path(dx, dy);
}

#if 0
int bored()
{
	return 0;
}
#endif
