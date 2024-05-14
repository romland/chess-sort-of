/*
 * Buffer of incoming/outgoing data.
 *
 * The reason we buffer at this layer is because we might implement a client
 * which does polling (eg. http or similar) of state and data.
 *
 */
# include "../include/chess.h"
# define Secure() (object_name(previous_object())=="/usr/chess/sys/server")

private string *incoming;
private string *outgoing;
private string version;

/*
 * We keep name as well as user-object since the 'object' might lose
 * its connection; and we might implement functionality to recover 
 * from that. And for that, we'd need a name.
 */
private string name;
private object user;
private int state;
private int client_type;

static void create(varargs int clone)
{
	client_type = CT_UNKNOWN;
	state =       ST_NONE;
	version  =    "0";
	incoming =    ({ });
	outgoing =    ({ });
	name =        NO_PLAYER;
	user =        nil;
}

void set_user(object u)
{
	if(!Secure()) error("no"); 
	
	user = u;
	name = u->query_name();
}

void set_version(string s)	{ if(!Secure()) error("no"); version = s; }
void set_state(int n)		{ if(!Secure()) error("no"); state = n; }
void set_client_type(int n)	{ if(!Secure()) error("no"); client_type = n; }
void add_incoming(string s)	{ if(!Secure()) error("no"); incoming += ({ s }); }
void add_outgoing(string s)	{ if(!Secure()) error("no"); outgoing += ({ s }); }

string get_name()			{ return name; }
int    get_state()			{ return state; }
int    get_client_type()	{ return client_type; }
object get_user()			{ return user; }


/*
 * Incoming is (to this object) things sent to us by the client
 */
string get_incoming()
{
	string s;

	if(!Secure()) error("no");

	if(!sizeof(incoming)) return "";

	s = incoming[0];

	/* Each message is only held until fetched */
	incoming -= ({ incoming[0] });

	return s;
}


/*
 * Outgoing is (to this object) things the server is about to send
 */
string get_outgoing()
{
	string s;

	if(!Secure()) error("no");

	if(!sizeof(outgoing)) return "";

	s = outgoing[0];

	/* Each message is only held until fetched */
	outgoing -= ({ outgoing[0] });

	return s;
}
