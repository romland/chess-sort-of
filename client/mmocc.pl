#!/usr/bin/perl
use strict;
use IO::Socket::INET;
use Data::Dumper;

my $DEBUG = 0;
my $MESSAGES = 32;

print "\x1b[0;37;40m\n";
system q{clear};

my %graphics = (
	s => [
		"```````",
		"```````",
		"```````",
		"```````",
		"```````",
	],
	b => {
		p => [
			"```````",
			"``( )``",
			"``/ \\``",
			"`(___)`",
			"```````",
		],
		r => [
			"`[_|_]`",
			"``[ ]``",
			"``[ ]``",
			"`(___)`",
			"```````"
		],
		b => [
			"``/\\\\``",
			"``\\ /``",
			"``( )``",
			"`(___)`",
			"```````",
		],
		k => [
			"``//\\``",
			"`/   )`",
			"`L/  [`",
			"`(___)`",
			"```````",
		],
		Q => [
			"``\\^/``",
			"``{ }``",
			"``{ }``",
			"`(___)`",
			"```````",
		],
		K => [
			"``<+>``",
			"``{ }``",
			"``{ }``",
			"`(___)`",
			"```````",
		]
	},
	w => {
		p => [
			"```````",
			"``(#)``",
			"``/#\\``",
			"`(###)`",
			"```````",
		],
		r => [
			"`[_|_]`",
			"``[#]``",
			"``[#]``",
			"`(###)`",
			"```````"
		],
		b => [
			"``/\\\\``",
			"``\\#/``",
			"``(#)``",
			"`(###)`",
			"```````",
		],
		k => [
			"``//\\``",
			"`/###)`",
			"`H/##[`",
			"`(###)`",
			"```````",
		],
		Q => [
			"``\\^/``",
			"``{#}``",
			"``{#}``",
			"`(###)`",
			"```````",
		],
		K => [
			"``<+>``",
			"``{#}``",
			"``{#}``",
			"`(###)`",
			"```````",
		]
	}
);

my $move = 0;
my %PIECES = ();
my @COLOR = ( ' ', 'b', 'w' );
my @TYPE  = ( ' ', 'p', 'r', 'b', 'k', 'Q', 'K' );

my $MY_PIECE;

my $XFILE = sprintf './mmocc-%d.tmp', time;

my $uid = shift || time;
my $pwd = shift;

my @messages = ();

print "using $XFILE for data exchange.\n";

my $init = 1;
my %owners;

sub mmocc_handle_piece {
	my ($X, $Y, $Color, $Type, $Owner, $Action) = @_;

	if ($Action eq '+') {
		$PIECES{"$X$Y"} = {
			X => $X,
			Y => $Y,
			Color => $COLOR[ $Color ],
			Type => $TYPE[ $Type ]
		};

		$Owner eq 'Noone' or $owners{$Owner} = 1;

		$MY_PIECE = $PIECES{"$X$Y"} if $Owner eq $uid;
	}

	if ($Action eq '-') {
#		$Owner =~ /Noone/ or undef $owners{$Owner};

		undef $PIECES{"$X$Y"};
		undef $MY_PIECE if $Owner eq $uid;
	}
}

my @XX = split //, 'ABCDEFGH';


sub mmocc_print {
	
	local $_;

	my $x;
	my $y;
	my $l;
	my $p;
	my $c;
	my $t;

	print "\x1b[1;1H";

	my ($s, $m, $h) = (localtime)[0..2];

	print '     ';
	for $x (0..7) {
		printf '%s      ', $XX[$x];
	}
	printf '  %d:%02d:%02d   #%d', $h, $m, $s, $move;
	print "\n";

	my @info = grep $owners{$_}, sort keys %owners;
	my @info2 = ();
	unshift @info, 'ONLINE';
	unshift @info, '';
	unshift @info, sprintf 'you are at %s%d', $XX[$MY_PIECE->{X}], 8 - $MY_PIECE->{Y} if $MY_PIECE;
	unshift @info, '' unless $MY_PIECE;
	push @info2, '';
	push @info2, '';
	push @info2, 'MESSAGES';
	push @info2, @messages;

	for $y (0..7) {
		for $l (0..4) {
			if ($l == 2) {
				printf '%d ', 8-$y;
			} else {
				print '  ';
			}

			for $x (0..7) {
				if ($p = $PIECES{"$x$y"}) {
					$c = $p->{Color};
					$t = $p->{Type};
					$_ = $graphics{$c}{$t}[$l];
				} else {
					$_ = $graphics{s}[$l];
				}

				s/`/ /g if (($x + $y) & 1);
				print;
			}

			if ($l == 2) {
				printf ' %d', 8-$y;
			} else {
				print '  ';
			}

			my $info = shift @info || '';
			chomp $info;
			printf '   %-15s', "\u$info";

			my $info = shift @info2 || '';
			chomp $info;
			printf '%-50s', $info if length $info < 50;

			print "\n";
		}
	}

	print '     ';
	for $x (0..7) {
		printf '%s      ', $XX[$x];
	}

	print "\n";
	print (' ' x50);
	print "\n";

	open POS, "> $XFILE\0";
	printf POS "%s%d\n", $XX[$MY_PIECE->{X}], 8 - $MY_PIECE->{Y} if $MY_PIECE;
	printf POS '', unless $MY_PIECE;
	close POS;
	
	print "\x1b[42;1H\n";
}


my %NX = ( A => 0, B => 1, C => 2, D => 3, E => 4, F => 5, G => 6, H => 7 );
my %NY = ( 8 => 0, 7 => 1, 6 => 2, 5 => 3, 4 => 4, 3 => 5, 2 => 6, 1 => 7 );

sub mmocc_receive_state {
	local $_ = shift;
	my $my_piece = $MY_PIECE;

#	undef $MY_PIECE;

#	%owners = ();

	mmocc_handle_piece split ',', $_ for /{([^{].*?)}/sg;
#	mmocc_print;

	++$move;

	add_message(':: You died') unless $MY_PIECE || $MY_PIECE eq $my_piece;
}

sub add_message {
	local $_ = shift;
	return if !$DEBUG && /DEBUG/;

	s/^\s*//g;
	s/\s*$//g;
	s/^\s*(>\s*)*//g;
	
	shift @messages if @messages > $MESSAGES;
	push @messages, $_;
}

sub mmocc_receive {
	my $conn = shift;

	while (<$conn>) {
		/say|tell|DEBUG|ERROR/ and add_message($_)
			or
		/{/ and mmocc_receive_state $_;

		if (!$DEBUG) {
			/DEBUG: disconnect: (\w*)/ and do { $owners{$1} = undef; add_message(":: \u$1 left the game")  };
			/DEBUG: connect: (\w*)/    and do { $owners{$1} = 1;     add_message(":: \u$1 entered the game") };
			/DEBUG: (\w*) won!(.*)/    and do { add_message(":: $1 won!"); add_message(":: restarting game") };
		}

		mmocc_print;
	}
}

sub mmocc_send {
	my $conn = shift;
	my ($F, $T, $FX, $FY, $TX, $TY);
	
	while (<>) {
		if (/^\$(.*)/g) {
			print $conn "$+\n";
			next;
		}
		if (/-/) {
			($F, $T) = split '-', $_;
		
			($FX) = $F =~ /(\w)/;
			($FY) = $F =~ /(\d)/;

			$FX = $NX{"\u$FX"};
			$FY = $NY{$FY};
		} else {
			$F = `cat $XFILE`;
			($FX) = $F =~ /(\w)/sg;
			($FY) = $F =~ /(\d)/sg;
			$FX = $NX{"\u$FX"};
			$FY = $NY{$FY};

#			$FX = $TX;
#			$FY = $TY;
			$T = $_;
		}

		($TX) = $T =~ /(\w)/;
		($TY) = $T =~ /(\d)/;
		
		$TX = $NX{"\u$TX"};
		$TY = $NY{$TY};
		
		local $_ = sprintf "{%s,%s,%s,%s}\n", $FX, $FY, $TX-$FX, $TY-$FY;
#		print "$_\n";
		print $conn $_;
	}
}

my $conn;
undef until $conn = new IO::Socket::INET( 'localhost:1601' );

print $conn "$uid\n$pwd\nchess 1\n";

fork() ? mmocc_receive $conn : mmocc_send $conn;

