#include "../include/chess.h"

inherit LIB_PIECE;

static int valid_move(int dx, int dy)
{
	if(dy && dx) 
		return 0;
	return ::has_clear_path(dx, dy);
}
