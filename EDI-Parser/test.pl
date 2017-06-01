#! /usr/bin/perl
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';
$^W = 1;
use Test;
BEGIN { plan tests => 1 };
use EDI::Parser qw(:all);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

$| = 1;

# These aren't proper tests yet ...

# The default test edi file is stored in the <DATA> portion of this
# perl script. To use a different file, set the EDI_FILE environment
# variable to the filename, eg:

#    EDI_FILE=~/mytest.edi make test

# By default, XML transaction set guidelines will be looked for in
# ${HOME}/tsg, and named according to the syntax, version and type of
# the message. Placing (uncompressed) XML tsg files from
# http://www.disintegration.org/~david/tsg/ in a ${HOME}/tsg/ directory
# should work, but you can overide the file by specifying a filename
# in the EDI_TSG environment variable, eg:

#    EDI_TSG=~/mytsg.xml make test


# If an EDI file is specified use that instead of the sample in the
# DATA section of this script:

if(defined $ENV{EDI_FILE}) {
    open(DATA, "< $ENV{EDI_FILE}") or die "open($ENV{EDI_FILE}): $!\n";
}


my $chars = 1;
my @chars = qw(segment composite tag element );
my @event = qw(interchange group transaction loop);
my @nowsp = qw(tag element);

my %event = map {$_=>1} (@event, $chars ? @chars : ());
my %nowsp = map {$_=>1} @nowsp;
my @stack;

my %tsg;
my $total  = 0;
my $nread  = 0;
my $indent = 0;
my $parser = new EDI::Parser(Handlers => { Start     => \&handle_start,
					   End       => \&handle_end,
					   #Char      => \&handle_char,
					   #Segment   => \&handle_segment,
					   Warning   => \&handle_warning,
					   Error     => \&handle_error,
					   Directory => \&handle_directory,
					   #Default   => \&handle_char,
				       },
			     #Style => 'Subs',
			     );

$parser->setHandlers(Segment => \&handle_segment) if !$chars;
$parser->setHandlers(Char    => \&handle_char)    if  $chars;



while(($nread = read(DATA, $buffer, 1024))) {
    $total += $parser->parse($buffer, 0);
    last if $parser->error;
}

if(defined $nread) {
    $parser->parse('', 1);
} else {
    warn "Couldn't read($!)";
}

printf "Processed %d bytes with status: %s.\n", $total, $parser->status;  
 

sub handle_error {
    my($parser, $code, $string) = @_;
    printf STDERR "ERROR: %s\n", $string;
}

sub handle_warning {
    my($parser, $code, $string) = @_;
    printf STDERR "WARNING: %s\n", $string;
}

sub handle_start {
    my($parser, $event, %parameters) = @_;
    my @keys = qw(Standard Code Description RecipientsId SendersId
		  MessageType);

    push(@stack, $event);

    return unless defined $event{$event};

    printf "%s<%s", " "x$indent, $event;
    
    #foreach (keys %parameters) {
    foreach (@keys){
	my $value = $parameters{$_};
	next unless defined $value;
	$value =~ s/^\s*|\s*$//g;
	printf " %s=\"%s\"", $_, $value if $value =~ /\S/;
    }

    printf ">";
    printf "\n" unless defined $nowsp{$event};
    
    $indent += 4;
}

sub handle_end {
    my($parser, $event, %parameters) = @_;
    pop(@stack);

    return unless defined $event{$event};

    $indent -= 4;

    print " "x$indent unless defined $nowsp{$event};
    
    printf "</%s>\n", $event;
}

sub handle_char {
    my($parser, $chars) = @_;
    print $chars;
}

sub handle_segment {
    my($parser, $segment, %parameters) = @_;
    my($e, $s, $value);

    printf "%s[%-3s]=", " "x$indent, $segment->code;
    for($e = 0; $e < $segment->elementCount; $e++) {
	print "+" if $e;
	for($s = 0; $s < $segment->subelementCount($e); $s++) {
	    $value = $segment->element([$e, $s]);
	    printf "[%s]", defined $value ? $value : "";
	}
    }

    printf " # %s", $segment->name if defined $segment->name;
    print "\n";
}    

sub handle_directory {
    my($parser, %parameters) = @_;
    my($file, $tsg);

    #while(my($key, $val) = each(%parameters)) { print "$key=$val\n" }

    if(defined $ENV{EDI_TSG}) {
	$file = $ENV{EDI_TSG};

    } else {
	if($parser->type == UNGTDI) {
	    $file = "$ENV{HOME}/tsg/tradacom";
	    
	} elsif($parser->type == X12) {
	    $file = sprintf "%s/tsg/%s_%s",
	    $ENV{HOME},
	    $parameters{MessageReleaseNumber},
	    $parameters{MessageType};
	    $file =~ tr/A-Z/a-z/;
	    
	} elsif($parser->type == EDIFACT) {
	    $file = sprintf "%s/tsg/%s%s",
	    $ENV{HOME},
	    $parameters{MessageVersionNumber},
	    $parameters{MessageReleaseNumber};
	    $file =~ tr/A-Z/a-z/;
	}
	
	return undef unless defined $file;
	
	$file .= (-f "$file.pyx") ? '.pyx' : '.xml';
    }

    return $tsg{$file} if defined $tsg{$file};

    #printf STDERR "%s\n", $file;
    
    if(!open(FILE, "< $file")) {
	warn "*** Couldn't open TSG '$file': $!\n";
	warn "*** Please read the comments about TSG files in this script!\n";
	return undef;
    }

    if($file =~ /\.pyx$/) {
	$tsg = $parser->pyxtsg(\*FILE);
    } else {
	$tsg = $parser->xmltsg(\*FILE);
    }
    close(FILE);
    
    return $tsg{$file} = $tsg;
    return undef;
}





















































# this is experimental


sub interchange {
    my($parser, $event, %p) = @_;
    printf "%s %s\n", $event, join(' ', map { "$_=$p{$_}" } keys %p);
}

sub interchange_ {
    my($parser, $event, %parameters) = @_;
    printf "/%s\n", $event;
}

sub segment {
    my($parser, $event, %p) = @_;
    printf "%s %s\n", $event, join(' ', map { "$_=$p{$_}" } keys %p);
}

sub segment_ {
    my($parser, $event, %parameters) = @_;
    printf "/%s\n", $event;
}


sub element {
    my($parser, $event, %p) = @_;
    printf "%s %s\n", $event, join(' ', map { "$_=$p{$_}" } keys %p);
}




sub __UNB {
    my($parser, $segment, %params) = @_;
    printf "*** %s\n", $segment->element([0,0]);
}


sub __UNH {
    my($parser, $segment, %params) = @_;
    printf "%s\n", $segment->element([0,0]);
    $parser->abort;
}







__END__;
UNA:+.? 
UNB+UNOA:1+5010000000000:14+5010011900016:14+010108:1200+2030++PRICAT
UNH+2030+PRICAT:D:96A:UN:EAN006
BGM+9+2030+9
DTM+137:20010115:102
RFF+PL:2030
DTM+171:20010115:102
RFF+CR:A1111
NAD+BY+5010011900016::9
NAD+SU+5010000000000::9++SUPPLIER
CUX+2:GBP:8
PGI+2
TOD+1++EXW
LOC+1+00
LIN+1+3+5010000000001:EN
IMD+F++TU:::JS CUPS 
MEA+PD+AAA:4
QTY+52:12
PRI+AAA:5.4:CT
DTM+194:20010115:102
LIN+2+3+5010000000002:EN
IMD+F++TU:::JS WINDOW CLEANER
MEA+PD+AAA:4
QTY+52:12
PRI+AAA:4.5:CT
DTM+194:20010115:102
LIN+31+3+5000000000031:EN
IMD+F++TU:::JS BUTTONS
MEA+PD+AAA:4
QTY+52:27
PRI+AAA:3.56:CT
DTM+194:20010115:102
LIN+32+3+5000000000032:EN
IMD+F++TU:::JS ARTICHOKES
MEA+PD+AAA:4
QTY+52:27
PRI+AAA:3.56:CT
DTM+194:20010115:102
UNT+37+2030
UNZ+1+2030
