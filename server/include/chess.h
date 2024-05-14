# include <kernel/kernel.h>

# define ABS(a)		(((a)<0) ? -(a) : (a))

# define NONE       0
# define PAWN       1
# define ROOK       2
# define BISHOP     3
# define KNIGHT     4
# define QUEEN      5
# define KING       6

# define BLACK      1
# define WHITE      2

# define CHESS			"/usr/chess"
# define CHESS_SERVER	CHESS + "/sys/server"

# define LIB			CHESS+"/lib"
# define DATA			CHESS+"/data"
# define SYS			CHESS+"/sys"

# define PLAYER_C		CHESS + "/data/player"
# define ITERATOR_C		CHESS + "/data/iterator"
# define BOARD_C		CHESS + "/data/board"

# define LIB_PIECE		CHESS + "/lib/piece"
# define LIB_PROTO		CHESS + "/lib/proto"
# define LIB_ITERATOR	CHESS + "/lib/iterator"
# define LIB_ITERATOROBJ CHESS + "/lib/iteratorobj"		/* ugly work-around */

# define Piece			object LIB_PIECE
# define Iterator		object LIB_ITERATOROBJ
# define Board			object LIB_BOARD

# define PROTO_ACK		"OK"
# define PROTO_ERROR	"ERROR"
# define PROTO_VERSION	"1"			/* protocol version */

# define ST_NONE        0
# define ST_SETUP       1
# define ST_PLAY        2

# define CT_UNKNOWN     0
# define CT_NORMAL      1
# define CT_POLLING     2

# define DEBUG(s)   if(this_user()) this_user()->message("DEBUG: " + s + "\n")
# define SDEBUG(s)   find_object("/usr/System/chess/binaryd")->message("DEBUG: " + s + "\n")

# define Types(n)	(({ "n/a", "pawn", "rook", "bishop", "knight", "queen", "king" })[n])
# define Colors(n)	(({ "invisible", "black", "white" })[n])

# define TRUSTED()	(sscanf(previous_program(), USR_DIR + "/chess/%*s") || sscanf(previous_program(), USR_DIR + "/System/%*s") || sscanf(previous_program(), "/kernel/%*s"))

# define SCORE_CREATED	0
# define SCORE_WINS		1
# define SCORE_LOSSES	2
# define SCORE_KILLS	3
# define SCORE_DEATHS	4
# define SCORE_MOVES	5
# define SCORE_ENTRIES	6
# define SCORE_SIZE		7

# define NO_PLAYER		"Noone"

