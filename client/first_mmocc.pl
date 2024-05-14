#!/usr/bin/perl
use strict;
use IO::Socket::INET;
use Data::Dumper;

my %PIECES = ();
my @COLOR = ( ' ', 'b', 'w' );
my @TYPE  = ( ' ', 'p', 'r', 'b', 'k', 'Q', 'K' );

my $MY_PIECE;

my $uid = shift;
my $pwd = shift;

sub mmocc_handle_piece {
	my ($X, $Y, $Color, $Type, $Owner, $Action) = @_;
	
	if ($Action eq '+') {
		$PIECES{"$X$Y"} = {
			X => $X,
			Y => $Y,
			Color => $COLOR[ $Color ],
			Type => $TYPE[ $Type ]
		};
		$MY_PIECE = $PIECES{"$X$Y"} if ($Owner eq $uid);
	}

	undef $PIECES{"$X$Y"} if ($Action eq '-');
}

my @XX = split //, 'ABCDEFGH';

sub mmocc_print {
	my $x;
	my $y;
	my $p;

	printf '  %s', $XX[$_] for 0..7;
	print "\n";

	for $y (0..7) {
		print 8 - $y;
		print ' ';

		print (($p = $PIECES{"$_$y"})
			? "$p->{Type}  "
			: '   ')
			for 0..7;

		print "\n  ";
		print (($p = $PIECES{"$_$y"}) && $p->{Color} eq 'w'
			? '^  '
			: '   ')
			for 0..7;
		
		print "\n";
	}

	printf "\nYou are at %s%d\n\n", $XX[$MY_PIECE->{X}], 8 - $MY_PIECE->{Y};
	
	open POS, "> .pos";
	printf POS "%s%d\n", $XX[$MY_PIECE->{X}], 8 - $MY_PIECE->{Y};
	close POS;
}

my %NX = ( A => 0, B => 1, C => 2, D => 3, E => 4, F => 5, G => 6, H => 7 );
my %NY = ( 8 => 0, 7 => 1, 6 => 2, 5 => 3, 4 => 4, 3 => 5, 2 => 6, 1 => 7 );

sub mmocc_receive_state {
	local $_ = shift;

	mmocc_handle_piece split ',', $_ for /{([^{].*?)}/sg;
	mmocc_print;	
}

sub mmocc_receive {
	my $conn = shift;

	while (<$conn>) {
		/say|DEBUG/ and print
			or
		/{/ and mmocc_receive_state $_;
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
			$F = `cat ./.pos`;
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
#		print $_;
		print $conn $_;
	}
}

my $conn;
undef until $conn = new IO::Socket::INET( 'crazyharry:2005' );

print $conn "$uid\n$pwd\nchess 1\n";

fork() ? mmocc_receive $conn : mmocc_send $conn;

