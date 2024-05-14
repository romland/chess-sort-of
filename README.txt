Note: It's 2024 and I just found this on an old disk. The code was written
in spring of 2005.

This is a Massive-Multiplayer-Online-Chess-Game (aka MMOCG)

The general idea is that it is NOT turn-based and that up to 32 players
control one chess-piece each. It's all about speed and team-play :)

Game-rules:
	- Normal chess rules apply to moving-abilities of pieces.
	- Moving the queen will render all chess-pieces for that team unmovable
	  for two seconds.
	- The rest should just make sense.
	- If a piece is not moved for 20 seconds, the server might move it (this
	  is excepting the king and (possibly) queen).
	- If a team's got more players than the other when a game starts: The 
	  last player(s) to enter are become pawns.

A game starts when:
	- A minimum of N (2/4?) players are connected

A game ends when:
	- When a team has failed to protect its king for ten seconds, this
	  counter will reset every time the other team manages to make a move
	  that takes them out of a threat. 

	  Or in other word: If your king remains checked for 10 seconds, you
	  have lost.

Supported commands on server (not client) in chess-mode:
	- reset
	  restarts the game
	- recompile
	  recompiles all server components (this will keep state of game intact
	  thanks to DGD)
	- say
	- emote
	- ...more?
	- quit

The content of this directory is according to the following structure:
	~/data/		types
	~/lib/		inheritables
	~/obj/		cloables
	~/sys/		daemons
	~/include/	headers
	~/old/		junk

Tech-stuff:
In ~System/initd.c you will find code that hands over connections to 
the "chess-user" (~/chess/obj/user.c)



Authors:
	JR & GS (www.buyways.nl)
