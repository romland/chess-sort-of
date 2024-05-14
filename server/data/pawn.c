#include "../include/chess.h"

inherit LIB_PIECE;

static int valid_move(int dx, int dy)
{
	int athome, strike, val;

	if(get_color() == BLACK) val = 1; else val = -1;

	if (dx == 0 && dy == 0)
		return 0;

#if 0
	if (dy != val)
		return 0;
#endif
	
	if(server->is_occupied(this_object(), dx, dy) && server->get_piece_at(dx, dy, this_object())->get_color() != get_color()) {
		strike = 1;
	}

	/* valid first double move (jump two) */
	athome = !get_moves();
	if(dy == (val*2) && !dx && athome && !strike && has_clear_path(dx, dy)) {
		return 1;
	}

	/* valid other move */
	if(dy == val && !dx && !strike) {
		return 1;
	}

	/* valid strike */
	if(strike && dy == val && (dx == 1 || dx == -1)) {
		return 1;
	}

	return 0;
}

#if 0
int move(int dx, int dy)
{
	if(::move(dx, dy)) {
		return 1;
	}

	return 0;
}
#endif

/*
int bored() { return 1; }
*/
