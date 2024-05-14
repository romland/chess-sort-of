/**
 * The order is relevant :
 *	- Destroy libraries
 *	- Recompile LWO's
 *	- Recompile Servers/clonables
 */

# include "../include/chess.h"

# define D(s)	destruct_object(s)
# define C(s)	compile_object(s)

string recompile()
{
	catch {
		/* lib/ */
		D(LIB+"/iterator");
		D(LIB+"/iteratorobj");
		D(LIB+"/piece");
		D(LIB+"/proto");

		/* data/ */
		C(DATA+"/bishop");
		C(DATA+"/board");
		C(DATA+"/iterator");
		C(DATA+"/king");
		C(DATA+"/knight");
		C(DATA+"/pawn");
		C(DATA+"/player");
		C(DATA+"/queen");
		C(DATA+"/rook");

		/* server */
		C(SYS+"/stats");
		C(SYS+"/server");
		C(SYS+"/recompile");
		return "Successful recompile.";
	} : {
		return "Compile error, check log.";
	}
}
