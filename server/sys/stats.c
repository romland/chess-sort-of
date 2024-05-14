# include "../include/chess.h"

int games;
mapping score;


private void load()
{
	restore_object("/usr/chess/save/stats");
}


private void save()
{
	save_object("/usr/chess/save/stats");
}


static void create(varargs int clone)
{
	if(clone) {
		return;
	}

	if(!score) {
		load();
		if(!score) {
			score = ([ ]);
		}
	}

}


private known(string player)
{
	if(!score)
		score = ([ ]);

	if(!score[player]) {
		score[player] = allocate(SCORE_SIZE);
		score[player][SCORE_CREATED] =	time();
		score[player][SCORE_WINS] =		0;
		score[player][SCORE_LOSSES] =	0;
		score[player][SCORE_KILLS] =	0;
		score[player][SCORE_DEATHS] =	0;
		score[player][SCORE_MOVES] =	0;
		score[player][SCORE_ENTRIES] =	0;
	}
}

void add_game()				{ games++; save(); }
void add_kill(string p)		{ known(p); score[p][SCORE_KILLS]++;	save(); }
void add_death(string p)	{ known(p); score[p][SCORE_DEATHS]++;	save(); }
void add_win(string p)		{ known(p); score[p][SCORE_WINS]++;		save(); }
void add_loss(string p)		{ known(p); score[p][SCORE_LOSSES]++;	save(); }
void add_move(string p)		{ known(p); score[p][SCORE_MOVES]++;	save(); }
void add_entry(string p)	{ known(p); score[p][SCORE_ENTRIES]++;	save(); }

mixed *get_score(string p)	{ return (score[p] ? score[p] + ({ }) : nil);	}
