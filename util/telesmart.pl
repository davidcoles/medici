#!/usr/bin/perl
if(!defined $VERSION) {
    exit TeleSmart::FTP::main(\*STDIN, \*STDOUT, $ARGV[1]) if $ARGV[0] eq '-f';
    exit TeleSmart::API::main();
}








# build various NETCMD commands for the TeleSmart API
# TODO: maybe -I -M -R -D -T for header fields?

package TeleSmart::API;
use Getopt::Std;
use strict;

sub usage { return <<EOF;
$0: [-u <user>] [-p <pass>] [ -asitd <val> ] <command> [ <command args> ... ]
    NCPASS <newpass>
    NAUTODEL <days>
    NRECEIVE <status>
    NDELETE <status>
    NLMAIL
    NLPOST
    -a <appref>
    -s <sender>
    -i <icr>
    -t <type>
    -d <date>
EOF
}

sub main {
    my($hook) = @_;
    my %opts;

    getopts('u:p:a:s:i:t:d:', \%opts) or die &usage;
    
    my $cmd = shift(@ARGV);
    
    die usage() unless defined $cmd &&
	$cmd =~ /^(NCPASS|NAUTODEL|NRECEIVE|NDELETE||NLMAIL|NLPOST)$/i;
    
    $cmd =~ tr/[a-z]/[A-Z]/;

    $opts{'p'} = &{$hook}($opts{'u'}) if defined $hook && !defined $opts{'p'};

    return command(\%opts, $cmd, @ARGV);
}


# build an interchange for a specific command (specified in $cmd)
# mandatory arguments (such as U/R for NRECEIVE/NDELETE in @args)
# optional arguments in $opts

sub command {
    my($opts, $cmd, @args) = @_;

    # get the time and a random number for later use ...
    my($SECONDS,$MINUTE,$HOUR,$DAY,$MONTH,$YEAR) = (localtime())[0,1,2,3,4,5];
    my $RAND = int(rand(100))%100;

    # determine content for various header fields
    my $FROM = defined $opts->{'u'} ? $opts->{'u'} : 'USERNAME';
    my $PASS = defined $opts->{'p'} ? $opts->{'p'} : 'PASSWORD';
    my $UNTO = 'EDINETWORK';
    my $DATE = sprintf "%02d%02d%02d", $YEAR%100, $MONTH+1, $DAY;
    my $TIME = sprintf "%02d%02d", $HOUR, $MINUTE;
    

    # build some ICR/MRN fields - doesn't seem to matter if the same
    # number is used again (no state between ftp sessions), but it
    # might be useful to know which message was which
    my $ICR = sprintf "%02d%02d%02d%02d%02d%02d%02d",
    $YEAR%100, $MONTH+1, $DAY, $HOUR, $MINUTE, $SECONDS, $RAND;
    my $MRN = sprintf "%02d%02d%02d%02d%02d%02d%02d",
    $YEAR%100, $MONTH+1, $DAY, $HOUR, $MINUTE, $SECONDS, $RAND;
    
    my $msg;
    

    # pass the arguments to produce the various messages (in $msg)
    if($cmd eq 'NCPASS') {
	my $newpass = shift(@args);
	$msg = NCPASS($MRN, $newpass);

    } elsif($cmd eq 'NAUTODEL') {
	my $days = shift(@args);
	$msg = NAUTODEL($MRN, $days);

    } elsif($cmd eq 'NRECEIVE' || $cmd eq 'NDELETE') {
	my $status = shift(@args);
	my $appref = $opts->{'a'};
	my $sender = $opts->{'s'};
	my $icr    = $opts->{'i'};
	my $type   = $opts->{'t'};
	my $date   = $opts->{'d'};

	die "$cmd status should be U or R\n" unless $status =~ /^[U|R]$/;
	
	if($cmd eq 'NDELETE') {
	    $msg = NDELETE  ($MRN,$status,$appref,$sender,$icr,$type,$date);
	} else {
	    $msg = NRECEIVE ($MRN,$status,$appref,$sender,$icr,$type,$date);
	}
	
    } elsif($cmd eq 'NLMAIL') {
	$msg = NLMAIL($MRN);

    } elsif($cmd eq 'NLPOST') {
	$msg = NLPOST($MRN);
    } else {
	die usage();
    }


    # print the header line, message and trailer to stdout
    print UNB($FROM, $UNTO, $DATE, $TIME, $ICR, $PASS, 'NETCMD');
    print $msg;
    print UNZ(1, $ICR);

    # huzzah!
    0;
}








# build each individual message type

sub NCPASS {
    my($MRN, $newpass) = @_;
    my $msg = '';
    $msg .= UNH($MRN, 'NETCMD', 2, 93);
    $msg .= BGM('255', 'NCPASS');
    $msg .= NPS($newpass);
    $msg .= UNT(4, $MRN);
    $msg;
}

sub NAUTODEL {
    my($MRN, $frd) = @_;
    my $msg = '';
    $msg .= UNH($MRN, 'NETCMD', 2, 93);
    $msg .= BGM('254', 'NAUTODEL');
    $msg .= FRD($frd);
    $msg .= UNT(4, $MRN);
    $msg;
}

sub NRECEIVE {
    my($MRN, $status, $appref, $sender, $icr, $type, $date) = @_;
    my $msg = '';
    $msg .= UNH($MRN, 'NETCMD', 2, 93);
    $msg .= BGM('252', 'NRECEIVE');
    $msg .= NCP($status, $appref, $sender, $icr, $type, $date);
    $msg .= UNT(4, $MRN);
    $msg;
}

sub NDELETE {
    my($MRN, $status, $appref, $sender, $icr, $type, $date) = @_;
    my $msg = '';
    $msg .= UNH($MRN, 'NETCMD', 2, 93);
    $msg .= BGM('253', 'NDELETE');
    $msg .= NCP($status, $appref, $sender, $icr, $type, $date);
    $msg .= UNT(4, $MRN);
    $msg;
}

sub NLPOST {
    my($MRN) = @_;
    my $msg = '';
    $msg .= UNH($MRN, 'NETCMD', 2, 93);
    $msg .= BGM('251', 'NLPOST');
    $msg .= UNT(3, $MRN);
    $msg;
}

sub NLMAIL {
    my($MRN) = @_;
    my $msg = '';
    $msg .= UNH($MRN, 'NETCMD', 2, 93);
    $msg .= BGM('250', 'NLMAIL');
    $msg .= UNT(3, $MRN);
    $msg;
}












# build individual segments from arguments
sub UNB {
    my($from, $to, $date, $time, $icr, $pass, $app) = @_;
    segment('UNB', ['UNOA', 2], $from, $to, [$date, $time], $icr, $pass, $app);
    
}
sub UNH {
    my($mrn, $msgtype, $msgver, $msgrel) = @_;
    segment('UNH', $mrn, [$msgtype, $msgver, $msgrel]);
}
sub BGM {
    my($msgcode, $msgname) = @_;
    segment('BGM', [$msgcode, undef, undef, $msgname]);
}
sub NPS {
    my($nps) = @_;
    segment('NPS', $nps);
}
sub FRD {
    my($frd) = @_;
    segment('FRD', $frd);
}
sub NCP {
    my($status, $appref, $sender, $icr, $type, $date) = @_;
    segment('NCP', $status, $appref, [$sender, $icr], $type, $date);
}
sub UNT {
    my($nsegs, $mrn) = @_;
    segment('UNT', $nsegs, $mrn);
}
sub UNZ {
    my($nmsgs, $icr) = @_;
    segment('UNZ', $nmsgs, $icr)
}









# build a segment from list of elements/subelements
sub segment {
    my $ST = "'";
    my(@elem) = @_;
    foreach my $elem (@elem) {
	if(ref($elem) eq 'ARRAY') {
            $elem = join(':', map { &release($_) } @$elem);
	} else {
            release($elem);
	}
    }    
     
    return join('+', @elem) . $ST;
}
 
# escape ("release") any special characters (?:+')
sub release {
    $_[0] = '' unless defined $_[0];
    $_[0] =~ s|\s| |g;
    $_[0] =~ s|\077|\077\077|g;
    $_[0] =~ s|\053|\077\053|g;
    $_[0] =~ s|\072|\077\072|g;
    $_[0] =~ s|\047|\077\047|g;
    $_[0];
}














































# transfer one or more interchanges for telesmart ftp from a file handle
# results stream is dumped to a second file handle

package TeleSmart::FTP;
use Net::FTP;
use EDI::Parser qw(:all);
use strict;
use bytes;

# BUFFER_SIZE must be big enough to get the whole first segment(UNB/STX)
# in, perhaps make it big enough to handle IO efficiently (4096/8192)

use constant BUFFER_SIZE => 4096;

sub main {
    my($cmd_fh, $ack_fh, $host, $hook) = @_;
    my $user = undef;
    my $pass = undef;
    my $buff = undef;
    my $read = undef;

    if(($read = read($cmd_fh, $buff, BUFFER_SIZE))) {
	
	# look at the EDI stream for potential login details
	($user, $pass) = getLoginCredentialsFromEDIStream($buff);
	
	# maybe get alternate login details
	($host, $user, $pass) = &{$hook}($host, $user, $pass) if defined $hook;
	
	
	# Check we have enough information to continue
	if(!defined $host) {
	    warn "No ftp host given!\n";
            return -1;
	}
	
	if(!defined $user || !defined $pass) {
	    warn "Insufficient login credentials for $host! ($user / $pass)\n";
	    return -1;
	}

	# setup network connection ...
	my $ftp = Net::FTP->new($host, Debug => 0);

	if(!defined $ftp) {
	    warn "Couldn't create Net::FTP object: $!\n";
	    return -1;
	}

	# login to the host
	if(!$ftp->login($user, $pass)) {
	    warn sprintf "Couldn't log in to '%s': %s\n", $host, $ftp->message;
	    $ftp->quit;
	    return -1;
	}

	# set binary or the ftp server may refuse to accept commands
	if(!$ftp->binary) {
	    warn "Couldn't set binary mode!\n";
	    $ftp->quit;
	    return -1;
	}
	
	# upload command.cmd from buffer we read earlier and filehandle
	if(uploadEDI($ftp, 'command.cmd', $buff, $cmd_fh)) {
	    # GET net.ack to filehandle
	    if(!defined($ftp->get('net.ack', $ack_fh))) {
		warn "Error downloading net.ack file!\n";
		$ftp->quit;
		return -1;
	    }
	} else {
	    warn "Error uploading command.cmd file!\n";
	    $ftp->quit;
	    return -1;
	}
	
	$ftp->quit;

    } else {
	if(defined $read) {
	    warn "Empty file on $cmd_fh!\n";
	} else {
	    warn "Error reading from $cmd_fh: $!\n";
	}

	return -1;
    }

    0;
}

sub uploadEDI {
    my($ftp, $name, $buffer, $fh) = @_;
    my $timeout = 30;
    
    return 0 unless defined $name;
    return 0 unless defined $fh;

    if(!$ftp->supported('STOR')) {
       warn "here!\n";
       return -1;
   }
	
    my $dc = $ftp->stou($name);
    
    return 0 unless defined $dc;
    
    my $length = length($buffer);
    
    do {
      write_buffer:
	my $wrote = $dc->write($buffer, $length, $timeout);
	
	if(!$wrote) {
	    warn "write failed in upload: $!\n";
	    $dc->close;
	    return 0;
	}
	
	if($wrote < $length) {
	    substr($buffer, 0, $wrote, '');
	    $length -= $wrote;
	    goto write_buffer;
	}
    } while($length = read($fh, $buffer, 8192));
	
    
    if(!defined $length) {
	warn "read failed in upload: $!\n";
	$dc->close;
	return 0;
    }
    
    return 0 unless $dc->close;

    1;
}
























sub getLoginCredentialsFromEDIStream {
    my($buffer) = @_;
    my $parser = new EDI::Parser(Handlers => { Start => \&handle_start });
    
    $parser->{_telesmart_done} = 0;

    $parser->parse($buffer, 0);
    
    return($parser->{_telesmart_user}, $parser->{_telesmart_pass});
}

sub handle_start {
    my($parser, $event, %params) = @_;
    
    if($event eq 'interchange' && !$parser->{_telesmart_done}) {
	
	# ensure this only gets run on the first interchange ...
	$parser->{_telesmart_done} = 1;
	
	$parser->{_telesmart_user} = $params{'SendersId'};
	
	if(defined $params{'ApplicationReference'} &&
	   $params{'ApplicationReference'} eq 'NETCMD') {
	    $parser->{_telesmart_pass} = $params{'RecipientsReferencePassword'}
	}
    }
}

1;
