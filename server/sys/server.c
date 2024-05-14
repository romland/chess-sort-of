# include "../include/chess.h"
# include <type.h>

inherit LIB_PROTO;

private object statd, bind;
private object board;
private int restarting;

static int dispatch_cmd(object user, string str);
static int move(Piece piece, int dx, int dy);
static int cmd_move(object user, int x, int y, int dx, int dy);
static int cmd_reset(object user);
static int cmd_recompile(object user);

#if 1
# define DEBUG_COM
#endif

#if 0
# define TURBO_GAME
#endif

/**
 * NAME: create()
 * DESC: Creator for chess server.
 * NOTE: 
 */
static void create(varargs int clone)
{
	if(clone)
		error("Can't be cloned.");

	statd   = find_object("/usr/chess/sys/stats");
	bind    = find_object("/usr/System/chess/binaryd");

	if(!bind || !statd)
		error("A daemon did not load");
	
	::create();
}


/**
 * NAME: setup_board()
 * DESC: Sets up a default board according to rules defined in the board.
 * NOTE: 
 */
static void setup_board()
{
	board = new_object(BOARD_C);
	board->setup(nil);	/* nil is default setup */

	if(!board) {
		error("Could not set up a board");
	}

	call_out("random_move", 1);

	statd->add_game();
}


object get_board()
{
	if(previous_object() != this_user() && !TRUSTED()) {
		return nil;
	}

	return board;
}


/**
 * NAME: get_piece_by_type()
 * DESC: Find a piece by color, type and id.
 * NOTE: 
 */
Piece get_piece_by_type(int color, int type, varargs int id)
{
	Iterator i;
	Piece p;

	i = board->iterator();

	while((p = i->next())) {
		if(p->get_color() == color && p->get_type() == 
		   type && p->get_id() == id) {
			return p;
		}
	}

	return nil;
}


/**
 * NAME: get_piece_at()
 * DESC: Returns 1 if square dx/dy from piece is occupied, 0 otherwise.
 * NOTE: If piece is passed to us, x and y are assumed to be deltas.
 */
Piece get_piece_at(int x, int y, varargs Piece p)
{
	if(p)
		return board->get(x, y, p);
	return board->get(x, y);
}


/**
 * NAME: is_occupied()
 * DESC: Returns 1 if square dx/dy from piece is occupied, 0 otherwise.
 * NOTE: 
 */
int is_occupied(Piece p, int dx, int dy)
{
	return board->is_occupied(p, dx, dy);
}


/**
 * NAME: is_checked()
 * DESC: Returns ({pieces}) if 'color' is currently checked.
 * NOTE: 
 */
Piece *is_checked(int color, varargs object tmpbrd)
{
	Piece p;
	Piece king;
	Piece *ret;
	Iterator i;

	int dx, dy;

	/* Ugly fix to enable us to check copies of a board */
	if(!tmpbrd) 
		tmpbrd = board;

	ret  = ({ });
	king = get_piece_by_type(color, KING, 1);

	if(!king) {
		error("The king is dead!  Long live the king!");
	}

	i = tmpbrd->iterator();
	while((p = i->next())) {
		dx = p->x() - king->x();
		dy = p->y() - king->y();
		if(p->get_color() != color && p->can_move_to(dx, dy)) {
			ret += ({ p });
		}
	}

	return ret;
}


/*
 * TODO:
 * 1. Can any of our pieces be moved to stand in the way of the king?
 * 2. Can any of our pieces take out the threat?
 * 3. Can we move the king to move away from the threat?
 */
int is_mated(int color)
{
	return 0;
}



/**
 * NAME: connect()
 * DESC: Set up a new user and return the current board to caller.
 * NOTE: 
 */
string connect(object user, varargs string version)
{
	object player;
	Piece piece;
	string name, s;

	if(!TRUSTED()) error("Illegal call");

	name = user->query_name();

	if(name == "noone")
		return PROTO_ERROR + ": Invalid name";

	if(!version)
		version = "1";

	if(version != PROTO_VERSION)
		return PROTO_ERROR + ": Version error (wants " + PROTO_VERSION + ")";

	if(!board)
		setup_board();

	/* Set up a new player object */
	player = new_player(user);
	player->set_version(version);

	/* Assign first available piece to the player */
	piece = get_piece_by_user(NO_PLAYER, board);
	if(piece) {
		piece->set_owner(name);
	} else {
		return PROTO_ERROR + ": No available pieces, bye bye";
	}

	DEBUG("You are: " + piece->to_friendly_string());

	player->set_state(ST_SETUP);
	s = board->to_string();

	statd->add_entry(name);

	send_to_all("DEBUG: connect: " + user->query_name());

	return s;
}


/**
 * NAME: disconnect()
 * DESC: Clean up after a user that's just been disconnected.
 * NOTE: 
 */
void disconnect(object user)
{
	Piece p;

	if(!TRUSTED())
		error("Illegal call");
	
	p = get_piece_by_user(user->query_name(), board);
	if(p) {
		p->set_owner(NO_PLAYER);
		remove_player(user->query_name());
	}

	send_to_all("DEBUG: disconnect: " + user->query_name());
}


/**
 * NAME: recieve()
 * DESC: Entry point for incoming data from a client (user).
 * NOTE: 
 */
void recieve(object user, string str)
{
	object player;

	if(!TRUSTED()) error("Illegal call");

	if(!(player = get_player(user))) {
		error("Unknown chess-player: " + user->query_name() );
	}

	if(player->get_client_type() == CT_POLLING) {
		player->add_incoming(str);

	} else if(!dispatch_cmd(user, str)) {
		send(user, "Unknown chess-command: " + str + "");
	}

}


/**
 * NAME: dispatch_cmd()
 * DESC: Parse incoming client commands and act on them.
 * NOTE: 
 */
static int dispatch_cmd(object user, string str)
{
	int i, x, y, dx, dy;
	Piece piece;
#ifdef DEBUG_COM
	bind->message("RCVD: " + str + " [" + user->query_name() + "]\n");
#endif
	if(sscanf(str, "{%d,%d,%d,%d}", x, y, dx, dy) == 4) {
		cmd_move(user, x, y, dx, dy);
		return 1;

	} else if(str == "reset") {
		cmd_reset(user);
		return 1;
	} else if(str == "recompile") {
		cmd_recompile(user);
		return 1;
	}

	return 0;
}


/**
 * NAME: cmd_move()
 * DESC: Make a move based on an incoming command from client.
 * NOTE: 
 */
static int cmd_move(object user, int x, int y, int dx, int dy)
{
	Piece piece;

	if(!(piece = board->get(x, y))) {
		send(user, PROTO_ERROR + ": No piece at position");
		return 0;
	}

	if(user->query_name() != piece->get_owner()) {
		send(user, PROTO_ERROR + ": You do not own that piece");
		return 0;
	}

	move(piece, dx, dy);
	return 1;
}


/**
 * NAME: cmd_reset()
 * DESC: Make a board-reset based on an incoming command from client.
 * NOTE: 
 */
static int cmd_reset(object user)
{
	object *pls;
	string name;
	int i;

	if(user) {
		name = user->query_name();

		if(name != "trick" && name != "giel") {
			send(user, PROTO_ERROR + ": You do not have permission for that");
			return 0;
		}

		send_to_all("Board reset by " + this_user()->query_name() + "\n");
	}

	restarting = 0;
	send_to_all( board->to_string("-") );

	board = nil;
	setup_board();
			
	pls = get_players();
	for(i = 0; i < sizeof(pls); i++) {
		connect(pls[i]->get_user());
	}

	send_to_all( board->to_string() );

	return 1;
}


/**
 * NAME: cmd_recompile()
 * DESC: Recompile all server components.
 * NOTE: 
 */
static int cmd_recompile(object user)
{
	string name;

	name = user->query_name();

	if(name != "trick" && name != "giel") {
		send(user, PROTO_ERROR + ": You do not have permission for that");
		return 0;
	}

	DEBUG( find_object("/usr/chess/sys/recompile")->recompile() );
	
	return 1;
}


static void restart_game()
{
	if(restarting)
		return;

	send_to_all("DEBUG: " + Colors(board->get_winner()) + " won! " +
			"Game is restarting!");

	call_out("cmd_reset", 10, nil);
	restarting = 1;
}


/**
 * NAME: random_move()
 * DESC: Randomly move a piece if it is idle and not a 'crucial' piece.
 * NOTE: This calls-out to itself, be careful.
 */
static int random_move()
{
	Iterator i;
	Piece p;
	Piece *pcs;
	int j, rx, ry, moved;
	int * pos;
	
	pos = ({ 0,7,1,1,6,6,2,2,5,5,3,3,3,3,3,4,4,4,4,4 });

	if(board->get_winner()) {
		SDEBUG( " random_move() determined winner!");
		SDEBUG( Colors(board->get_winner()) + " won!");
		restart_game();
		return 0;
	}
	
	i = board->iterator();

	pcs = ({ });

	while((p = i->next()))
#ifndef TURBO_GAME
		if(p->bored())
			pcs += ({ p });
#else
		pcs += ({ p });
#endif
	if(sizeof(pcs)) {
		p = pcs[random(sizeof(pcs))];
		
		if(!p->is_killed()) {
			if (p->type() == PAWN) {
				int dx1,dx2,dy1,dy2,T;
			
				dx1 = dy1 =  1;
				dx2 = dy2 = -1;
			
				if (random(4) < 2) { T = dx1; dx1 = dx2; dx2 = T; }
				if (random(4) < 2) { T = dy1; dy1 = dy2; dy2 = T; }
			
				if (p->can_move(p, dx1, dy1)  &&  move(p, dx1, dy1)) moved = 1; else
				if (p->can_move(p, dx1, dy2)  &&  move(p, dx1, dy2)) moved = 1; else
				if (p->can_move(p, dx2, dy1)  &&  move(p, dx2, dy1)) moved = 1; else
				if (p->can_move(p, dx2, dy2)  &&  move(p, dx2, dy2)) moved = 1;
			}

			for(j = 0; j < 50 && !moved; j++) {
				rx = pos[random(20)] - p->x();
				ry = pos[random(20)] - p->y();

				if(p->can_move_to(rx, ry)) {
					if(move(p, rx, ry)) moved = 1;
				}
			}
		} else {
			SDEBUG("random_move() : p->is_killed() hmm -- why is it on board?");
		} 
	}

#ifndef TURBO_GAME
	if(!moved && sizeof(pcs)) {
		call_out("random_move", 0.5);
	} else {
		call_out("random_move", 20);
	}
#else
	call_out("random_move", 0.1);
#endif

	return moved;
}


/**
 * NOTE: Color of piece is the color/king of interest.
 */
static int is_unchecking(Piece piece, int dx, int dy)
{
	object copy;

	copy = board->copy();

	piece = copy->get(piece->x(), piece->y());	/* get "same" piece from copy */

	if(!piece) {
		SDEBUG("board : " + board->to_string());
		SDEBUG(" copy : " + copy->to_string());
		error("Inconsistency between copy and board");
	}

	copy->move(piece, dx, dy);

	if(sizeof(is_checked(piece->get_color(), copy))) {
		SDEBUG("Not an unchecking move...");
		return 0;
	}

	return 1;
}


/**
 * NAME: move()
 * DESC: Move piece to dx, dy and send updates to affected clients.
 * NOTE: Returns 1 if the piece was moved.
 */
static int move(Piece piece, int dx, int dy)
{
	int i, x, y;
	Piece *affected, *threats;
	string data, old_pos;

	/* Hack to stop cheaters that just paste in commands */
	if(this_user() && !piece->get_idle()) {
		DEBUG("move() : Cheater!");
		return 0;
	}

	if(sizeof((threats = is_checked(WHITE)))) {
		DEBUG("White is checked by: ");
		for(i = 0; i < sizeof(threats); i++)
			DEBUG("\t\t" + threats[i]->to_friendly_string() );
		board->set_threat(WHITE);

		if(board->next_to_move() != (BLACK+WHITE) && piece->get_color() == BLACK && !is_unchecking(piece, dx, dy)) {
			SDEBUG("That move is invalid as it will not protect White's king.");
			return 0;
		} else {
			board->clear_threat(WHITE);
		}
	} else {
		board->clear_threat(WHITE);
	}

	if(sizeof((threats = is_checked(BLACK)))) {
		DEBUG("Black is checked by: ");
		for(i = 0; i < sizeof(threats); i++)
			DEBUG("\t\t" + threats[i]->to_friendly_string() );
		board->set_threat(BLACK);

		if(board->next_to_move() != (BLACK+WHITE) && piece->get_color() == WHITE && !is_unchecking(piece, dx, dy)) {
			SDEBUG("That move is invalid as it will not protect Black's king.");
			return 0;
		} else {
			board->clear_threat(BLACK);
		}
	} else {
		board->clear_threat(BLACK);
	}

	if(board->get_winner()) {
		/* TODO: Update stats for player... */
		DEBUG("Sorry! " + Colors(board->get_winner()) + " already won!");
		restart_game();
		return 0;
	}

	if(board->next_to_move() != piece->get_color() && board->next_to_move() != (BLACK+WHITE)) {
		DEBUG("Not your turn to move.");
		return 0;
	}
	
	old_pos = piece->to_string();
	x = piece->x();
	y = piece->y();

	/* Do the actual move. */
	affected = board->move(piece, dx, dy);
	if(!sizeof(affected)) {
		if(this_user()) 
			this_user()->message(PROTO_ERROR + ": Invalid move\n");
		return 0;
	}

	if(this_user())
		statd->add_move(this_user()->query_name());

	/* Remove piece from current (old; move() moved) position */
	board->set(x, y, nil);
	data = "{";
	data += "{";
	data += old_pos;
	data += ",-}";

	for(i = 0; i < sizeof(affected); i++) {
		data += "{" + affected[i]->to_string();

		if(affected[i]->is_killed()) {
			data += ",-";

			statd->add_death(affected[i]->get_name());

			if(this_user()) 
				statd->add_kill(this_user()->query_name());
		} else {
			data += ",+";
		}

		data += "}";
	}

	data += "}";

	send_to_all(data);

	return 1;
}
