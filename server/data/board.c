# include "../include/chess.h"

inherit LIB_ITERATOR;
inherit LIB_PROTO;

private Piece **board;		/* black is at 0, 0... */
private Piece *pieces;

private int white_king_threatened;
private int black_king_threatened;
private int moves;
private int turn_based;

Piece new_piece(int type, int color, int x, int y, int id);
Piece *move(Piece p, int dx, int dy);

Iterator iterator()		{ return new_object(ITERATOR_C);	}
Piece Iter_get(int i)	{ return pieces[i];					} 
int	Iter_size()			{ return sizeof(pieces);			}


void setup(Piece *pcs)
{
	int i, col;

	pieces = ({ });
#if 1
	turn_based = 0;
#endif

	board = allocate(8);
	for(i = 0; i < 8; i++)
		board[i] = allocate(8);

	if(pcs) {
		Piece p;

		for(i = 0; i < sizeof(pcs); i++) {
			p = pcs[i];
			new_piece(p->get_type(), p->get_color(), p->x(), p->y(), p->get_id());
		}

	} else {
		/* Place pieces in order we want to assign them to players */
		new_piece(KNIGHT,	BLACK,	1, 0, 1);
		new_piece(KNIGHT,	WHITE,	1, 7, 1);
		new_piece(KNIGHT,	BLACK,	6, 0, 2);
		new_piece(KNIGHT,	WHITE,	6, 7, 2);
		new_piece(QUEEN,	BLACK,	3, 0, 1);
		new_piece(QUEEN,	WHITE,	3, 7, 1);
		new_piece(ROOK,		BLACK,	0, 0, 1);
		new_piece(ROOK,		WHITE,	0, 7, 1);
		new_piece(ROOK,		BLACK,	7, 0, 2);
		new_piece(ROOK,		WHITE,	7, 7, 2);
		new_piece(BISHOP,	BLACK,	2, 0, 1);	
		new_piece(BISHOP,	WHITE,	5, 7, 2);
		new_piece(BISHOP,	BLACK,	5, 0, 2);	
		new_piece(BISHOP,	WHITE,	2, 7, 1);

		for(i = 0; i < 16; i++) {
			col = 1 + (i & 1);
			new_piece(PAWN, col,  (i ? i/2 : i), (col == WHITE ? 6 : 1), (i ? i/2 : i)+1);
		}

		new_piece(KING,		WHITE,	4, 7, 1);
		new_piece(KING,		BLACK,	4, 0, 1);
	}
}

static void create(varargs int clone)
{
}


void set_threat(int c)
{
	if(c == BLACK && !black_king_threatened) black_king_threatened = time();
	if(c == WHITE && !white_king_threatened) white_king_threatened = time();
}


void clear_threat(int c)
{
	if(c == BLACK && !black_king_threatened) black_king_threatened = 0; 
	if(c == WHITE && !white_king_threatened) white_king_threatened = 0;
}


int get_threat(int c)
{
	if(c == BLACK) return black_king_threatened;
	if(c == WHITE) return white_king_threatened;
}


/*
 * returns 0 if there is no winner yet
 */
int get_winner()
{
	if(turn_based)
		return 0;	/* Winner is not determined in turn-based */

	if(black_king_threatened == white_king_threatened)
		return 0;	/* It's still a draw, let game continue */

	if(black_king_threatened && (time() - black_king_threatened) > 9)
		return WHITE; /* White won */

	if(white_king_threatened && (time() - white_king_threatened) > 9)
		return BLACK; /* Black won */

	return 0;
}


int set(int x, int y, Piece p)
{
	/* This is quite ugly, but ... */
	if(p == nil) {
		pieces -= ({ p });
	}

	board[x][y] = p;
}


/* If piece is passed in, x and y are assumed to be deltas */
Piece get(int x, int y, varargs Piece p)
{
	if(p)
		return board[x + p->x()][y + p->y()];
	return board[x][y];
}


Piece new_piece(int type, int color, int x, int y, int id)
{
	Piece p;

	p = new_object("/usr/chess/data/" + Types(type));
	p->new(type, color, x, y, id);
	set(x, y, p);
	pieces += ({ p });

	p->start();

	return p;
}


int start()
{
	Iterator i;
	Piece p;

	i = iterator();
	while((p = i->next())) {
		p->start();
	}

	return 1;
}


int get_moves()
{
	return moves;
}


int next_to_move()
{
	if(turn_based)
		return (get_moves() & 1) ? WHITE : BLACK;
	return WHITE + BLACK;
}


string to_string(varargs string cmd)
{
	return ::board_to_data(board, cmd);
}


int is_occupied(Piece p, int dx, int dy)
{
	dx += p->x();
	dy += p->y();

	if (dx < 0 || 8 <= dx) { SDEBUG(p->to_friendly_string() + ": x ("+ dx +") out of range..."); return 0; }
	if (dy < 0 || 8 <= dy) { SDEBUG(p->to_friendly_string() + ": y ("+ dy +") out of range..."); return 0; }
	
	return !!(board[dx][dy]);
}


/**
 * NAME: move()
 * DESC: Returns, by a move, affected pieces in an array.
 * NOTE:
 */
Piece *move(Piece p, int dx, int dy)
{
	int x, y;
	Piece o;

	x = p->x() + dx;
	y = p->y() + dy;

	if(p->move(dx, dy)) {
		moves++;

#if 1 /* QUEEN CONVERSION */
		if(p->get_type() == PAWN && (y == 7 || y == 0)) {
			/* "Convert" me to queen! */
			object server, board, q;
			int id;

			server = find_object(CHESS_SERVER);
			server->send_to_all("queen conversion!");
			while(server->get_piece_by_type( p->get_color(), QUEEN, id)) {
				id++;
			}

			set(x-dx, y-dy, nil);
			q = new_piece(QUEEN, p->get_color(), x, y, id);
			q->set_owner(p->get_owner());
			p = q;
		}
#endif /* QUEEN CONVERSION */

		if((o = get(x, y)) && o && o != p) {
			if(o->get_color() != p->get_color()) {
				o->kill();
				set(x, y, p);
				return ({ o, p });
			} else {
				/*
				 * This should be checked prior to getting to this point.
				 * If piece strikes its own color here, something is buggy.
				 */
				error("This should never happen [see comment]");
			}
		}

		set(x, y, p);
		return ({ p });
	}

	return ({ });
}


object copy()
{
	int i;
	object o, *pcs;

	o = new_object(BOARD_C);
	
	pcs = ({ });

	for(i = 0; i < sizeof(pieces); i++) {
		pcs += ({ pieces[i]->copy() });
	}

	o->setup(pcs);
	
	return o;
}
