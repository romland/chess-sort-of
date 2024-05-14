# include "../include/chess.h"

int _x, _y, _color, _type, _moves, _dead, _id, _beat;
string _owner;
object server;

int		get_idle()			{ return time() - _beat; }

void	set_owner(string s)	{ _owner = s; }
string	get_owner()			{ return (_owner ? _owner : NO_PLAYER); } 

void	set_color(int x)	{ _color = x; } 
int		get_color()			{ return _color; } 

int		x()					{ return _x; } 
int		y()					{ return _y; }

int		get_moves()			{ return _moves; }

int		get_type()			{ return _type; }
void	set_type(int t)		{ _type = t; }

int		get_id()			{ return _id; }
int		set_id(int t)		{ _id = t; }

void	set_pos(int x, int y) { _x = x; _y = y; }

void new(int type, int color, int x, int y, int id)
{
	_type = type;
	_color = color;
	_x = x;
	_y = y;
	_moves = 0;
	_id = id;
	_owner = NO_PLAYER;

	server = find_object(CHESS_SERVER);

	if(!server) {
		error("Ahem. We have no server? Barfing!");
	}
}


int bored()
{
	return !_dead && get_idle() > 30 + random(60);
}


# define CHESS_COLS		"ABCDEFGH"
# define CHESS_ROWS		"87654321"
string get_chess_pos(int x, int y)
{
	return CHESS_COLS[x..x] + CHESS_ROWS[y..y];
}


string get_pos()
{
	return _x + "," + _y;
}


int has_clear_path(int dx, int dy)
{
	int x, y;
	int ix, iy;
	Piece p;

	p = this_object();
	
	if (dx < 0) ix = 1; else if (0 < dx) ix = -1;
	if (dy < 0) iy = 1; else if (0 < dy) iy = -1;

	x = p->x() + dx + ix;
	y = p->y() + dy + iy;

	for (;;) {
		if (x == p->x() && y == p->y())
			return 1;
#if 0
		if (board[x + y*8])
			return 0;
		if (server->is_occupied(p, dx, dy))
#else
		/*
		 * DGD oddness? If I omitted this_object() (which is wrong, and yes
		 * it was an error). But the line-number I got back in the stack-
		 * trace was WAY off. Said something like line 7411. Check if this
		 * is reproducable and report bug/ask about?
		 */
		dx = x - p->x();
		dy = y - p->y();

		/* NOTE: dx and dy are not deltas at this point! */
		if (7 < ABS(dx) || 7 < ABS(dy) || server->is_occupied(p, dx, dy))
			return 0;
#endif
		x += ix;
		y += iy;
	}

	error("???");
}


int can_move_to(int dx, int dy)
{
	Piece target;
	int x, y;

	if(dx == 0 && dy == 0) {
		return 0;
	}

	x = _x + dx;
	y = _y + dy;

	if(x < 0 || x > 7 || y < 0 || y > 7) {
		return 0;
	}

	target = server->get_piece_at(x, y);

	if(target && target->get_color() == _color) {
		return 0;
	}

	if(!this_object()->valid_move(dx, dy)) {
		return 0;
	}

	return 1;
}


int start()
{
	_beat = time();
}


int move(int dx, int dy)
{
	Piece target;
    int x, y;

	/* TODO: Security: Allow calls from server only */

	if(dx == 0 && dy == 0) {
		DEBUG("move() : Nothing moved");
		return 0;
	}

	x = _x + dx;
	y = _y + dy;

	if(x < 0 || y < 0 || x > 7 || y > 7) {
		DEBUG("move() : Out of boundaries");
		return 0;
	}

	target = server->get_piece_at(x, y);

	/*
	 * This check is done in the server as well, but we really do want
	 * to stop execution if this happens. In order to fix this double
	 * check we'd have to split the move-function into two. Some other
	 * day maybe :)
	 */
	if(target && target->get_color() == _color) {
		DEBUG("move() : Target at " + x + "," + y + " is your color");
		return 0;
	}

	if(target && target->get_type() == KING) {
		DEBUG("move() : You cannot strike the king");
		return 0;
	}

	if(!this_object()->valid_move(dx, dy)) {
		DEBUG("move() : Chess rules said 'no-no'");
		return 0;
	}

	_x += dx;
	_y += dy;
	_moves++;
	_beat = time();

	return 1;
}


void kill()
{
	_dead = 1;
}

int is_killed()
{
	return _dead;
}

string to_string()
{
	return get_pos()+","+get_color()+","+get_type()+","+get_owner();
}

string to_friendly_string()
{
	return Colors(get_color()) + " " + Types(get_type()) + " #" + get_id() + 
		" at " + get_chess_pos(_x, _y) + " (" + get_pos() + ")";
}

Piece copy()
{
	object o;

	o = new_object("/usr/chess/data/" + Types(_type));

	o->new(_type, _color, _x, _y, _id);
	o->set_owner(_owner);

	return o;
}
