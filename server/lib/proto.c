# include "../include/chess.h"
# include <type.h>

/*
 * client					server
 * ----------------------------------------------------------------
 *					<-		Username:
 *	playername\n	->	
 *					<-		Password:
 *	password\n		->
 *					<-		"> "
 *	"chess" <ver>	->
 *					<-		<the board> or ERROR (if version was not okay)
 *							{MT, {x,y,color,piece,+ / -} ... }
 *
 *	<client makes a move>
 *  {MT,x,y,dx,dy}	->
 *
 * <server notifies client of a move>
 *					<-		{MT, {x,y,color,piece,+ / -} ... }
 *	
 *
 *	Message-types (MT):
 *		100	=	Board Update [server only]
 *		101	=	Position Update [server only]
 *		200	=	Move Request [client only]
 *		300	=	Client Error (non-critical: invalid move, cannot strike king, etc) [server only]
 *		400	=	Player Message (say, tell, team) [server and client]
 *		500	=	Game Event (enter/leave game, white won, you are checked, game restarting etc) [server only]
 *		600	=	Server Error [server only]
 *		700	=	Debug Message [server only]
 *		800	=	Configuration Option [server and client]
 *		801	=	Configuration Option Request [client]
 *
 *	Message-type: "Board Update"
 *		a) type:
 *			100
 *		b...) position update(s):
 *			see Message-type "position update"
 *
 *		Example: {100, {101, ...}, {101, ...}, {101, ...}}
 *
 *	Message-type: "Position Update"
 *		a) type:
 *			101
 *		b) x-position
 *			0-7
 *		c) y-position
 *			0-7
 *		d) color
 *			1	=	black
 *			2	=	white
 *		e) piece-type
 *			1	=	Pawn
 *			2	=	Rook
 *			3	=	Bishop
 *			4	=	Knight
 *			5	=	Queen
 *			6	=	King
 *		f) action to take
 *			+	=	add piece
 *			-	=	remove piece
 *		g) player-name
 *
 *		Example: {101, x, y, color, piece, + / -, playername}
 *
 *	Message-type: "Move Request"
 *		a) type:
 *			200
 *		b) x-position
 *			0-7
 *		c) y-position
 *			0-7
 *		d) delta-x
 *		e) delta-y
 *
 *		Example: {200, x, y, dx, dy}
 *	
 *	Message-types: "Client Error", "Game Event", "Server Error" and "Debug Message"
 *		a) type:
 *			300 or 500 or 600 or 700
 *		b) source:
 *			always "Noone"
 *		c) target:
 *			1	=	all
 *			2	=	your team
 *			3	=	you
 *		d) ID of string:
 *			Pre-determined ID of the string so that client can replace message if
 *			it chooses to.
 *		e) String server thinks it should be, client can disregard it and print
 *		   its own message by looking at string-ID. The format of string is in
 *		   printf-kind-of-style
 *		f) varargs... attributes for argument [e]
 *
 *		Example: {300,"Noone",3,id,"Invalid move"}
 *		Example: {500,"Noone",1,id,"%s enters the game", "trick"}
 *		Example: {500,"Noone",1,id,"%s won", "black"}
 *		Example: {600,"Noone",1,id,"Compile error. Bye!"}
 *		Example: {700,"Noone",1,id,"yada yadebug"}
 *
 *	Message-type: "Player-Messenging"
 *		a) type:
 *			400
 *		b) source:
 *			player-name
 *		c) target:
 *			1	=	all
 *			2	=	your team
 *			3	=	you
 *		d) ID of string:
 *			n/a in this context, but kept for simplicity due to other types
 *		e) The actual string
 *
 *		Example: {400, "trick", 1, 0, "Good luck to everyone!"}
 *
 *	Message-type: "Configuration Option"
 *		a) type:
 *			800
 *		b) key
 *		c) value
 *
 *		Example: {800, "turn-based board", "enabled"}
 *
 *	Message-type: "Configuration Option Request"
 *		a) type:
 *			801
 *		b) key
 *
 *		Example: {801, "turned-based board"}
 */

/*
 * Player-data storage ([ name : pl-obj ])
 */
private mapping players;

static object get_player(mixed user);


static void create()
{
	players = ([ ]);
}


static string piece_to_data(Piece p)
{
	return p->to_string();
}


static object new_player(object user)
{
	object player;
	
	player = new_object(PLAYER_C);
	players[user->query_name()] = player;
	player->set_user(user);

	return player;
}


static void remove_player(string name)
{
	players[name] = nil;
}


static object* get_players()
{
	return map_values(players);
}


/**
 * NAME: get_piece_by_user()
 * DESC: Find a piece owned by a user.
 * NOTE:
 */
static Piece get_piece_by_user(string user, object board)
{
	Iterator i;
	Piece p;

	i = board->iterator();

	while((p = i->next())) {
		if(p->get_owner() == user) {
			return p;
		}
	}

	return nil;
}


/**
 * NAME: send()
 * DESC: Send or buffer (for polling) data to/for a user.
 * NOTE:
 */
static void send(object user, string str)
{
	object player;

	player = get_player(user);
	if(!player) {
		SDEBUG("send() : Player not found (Logged out? Fix disconnect())");
		return;
	}

	if(player->get_client_type() == CT_POLLING) {
		player->add_outgoing(str + "\n");
	} else {
		user->message(str + "\n");
#ifdef DEBUG_COM
		SDEBUG("SENT: " + str + " [" + user->query_name()  + "]");
#endif
	}
}


/**
 * NAME: send_to_all()
 * DESC: Send data to all currently connected users.
 * NOTE:
 */
void send_to_all(string str)
{
	int i;
	object *ps;

	if(!TRUSTED()) {
		error("illegal call");
	}

	ps = map_values(players);

	for(; i < sizeof(ps); i++) {
		send(ps[i]->get_user(), str);
	}
}

/**
 * NAME: send_to_team()
 * DESC: Send data to all users on specified team.
 * NOTE:
 */
static void send_to_team(int color, string str, object board)
{
	int i;
	object *ps, p;

	ps = map_values(players);

	for(; i < sizeof(ps); i++) {
		p = get_piece_by_user(ps[i]->get_user(), board);
		if(p && p->get_color() == color)
		send(ps[i]->get_user(), str);
	}
}


/**
 * NAME: poll_state()
 * DESC: Poll state for a user.
 * NOTE: For polling clients, it will take a command or a 'nop' and
 *       process this and in return the user will get all buffered
 *       data since last fetch.
 *
 *       This function has never been tested (!)
 */
string poll_state(object user, string cmd, string arg)
{
	int i;
	object player;
	string data, s;

	if(!TRUSTED()) error("Illegal call");

	player = players[user->query_name()];
	if(!player) {
		return PROTO_ERROR + ": No such player registered";
	}

#if 0
	if(cmd != "nop") {
		dispatch_cmd(object user, string str);
	}
#endif
	data = "";
	while((s = player->get_outgoing())) {
		data += s;
	}

	return data;
}


/**
 * NAME: get_player()
 * DESC: Find a player-object related to a user.
 * NOTE:
 */
static object get_player(mixed user)
{
	if(typeof(user) == T_OBJECT)
		return players[user->query_name()];

	if(typeof(user) == T_STRING)
		return players[user];

	error("invalid type for argument 1 [probably bug in disconnect, this]");
}

static string board_to_data(Piece **b, varargs string cmd)
{
	int i, j;
	string data;

	if(!cmd) {
		cmd = "+";
	}

	data = "{";

	for(; i < 8; i++) {
		for(j = 0; j < 8; j++) {
			if(b[i][j]) data += "{" + piece_to_data(b[i][j]) + "," + cmd + "}";
		}
	}

	data += "}";
	
	return data + "\n";
}
/*
 *		100	=	Board Update [server only]
 *		101	=	Position Update [server only]
 *		200	=	Move Request [client only]
 *		300	=	Client Error (non-critical: invalid move, cannot strike king, etc) [server only]
 *		400	=	Player Message (say, tell, team) [server and client]
 *		500	=	Game Event (enter/leave game, white won, you are checked, game restarting etc) [server only]
 *		600	=	Server Error [server only]
 *		700	=	Debug Message [server only]
 *		800	=	Configuration Option [server and client]
 *		801	=	Configuration Option Request [client]
 */
# define MT_BOARDUPDATE		100
# define MT_POSITIONUPDATE	101
# define MT_MOVEREQUEST		200
# define MT_CLIENTERROR		300
# define MT_PLAYERMESSAGE	400
# define MT_GAMEEVENT		500
# define MT_SERVERERROR		600
# define MT_DEBUGMESSAGE	700
# define NT_OPTION			800
# define MT_OPTIONREQUEST	801

# define M(VAR)	(typeof(VAR) == T_STRING ? "\"" + VAR + "\"" : VAR)

/**
 *	Message-types: "Client Error", "Game Event", "Server Error" and "Debug Message"
 *		a) type:
 *			300 or 500 or 600 or 700
 *		b) source:
 *			always "Noone"
 *		c) target:
 *			1	=	all
 *			2	=	your team
 *			3	=	you
 *		d) ID of string:
 *			Pre-determined ID of the string so that client can replace message if
 *			it chooses to.
 *		e) String server thinks it should be, client can disregard it and print
 *		   its own message by looking at string-ID. The format of string is in
 *		   printf-kind-of-style
 *		f) varargs... attributes for argument [e]
 *
 *		Example: {300,"Noone",3,id,"Invalid move"}
 *		Example: {500,"Noone",1,id,"%s enters the game", "trick"}
 *		Example: {500,"Noone",1,id,"%s won", "black"}
 *		Example: {600,"Noone",1,id,"Compile error. Bye!"}
 *		Example: {700,"Noone",1,id,"yada yadebug"}
 */
private string mt_message(int type, string msg, varargs int id)
{
	return "{" + type + "}";
}


string mt_boardupdate()
{
}

string mt_positionupdate() {}

string mt_clienterror(string msg, int id)
{
	return mt_message(MT_CLIENTERROR, msg);
}

/**
 *	Message-type: "Player-Messenging"
 *		a) type:
 *			400
 *		b) source:
 *			player-name
 *		c) target:
 *			1	=	all
 *			2	=	your team
 *			3	=	you
 *		d) ID of string:
 *			n/a in this context, but kept for simplicity due to other types
 *		e) The actual string
 *
 *		Example: {400, "trick", 1, 0, "Good luck to everyone!"}
 */
string mt_playermessage() {}
string mt_gameevent() {}
string mt_servererror() {}
string mt_debugmessage() {}
string mt_option() {}

