#! /usr/bin/perl
package EDI::Parser;

# Comment out the next three lines and remove the instances of "our "
# at the beginning of declarations to get this to work with older Perl
# (like 5.00405 which ships with IRIX)
use 5.006;
use strict;
use warnings;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader EDI::Parser::MEDICI);


# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use EDI::Parser ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw( EDIFACT UNGTDI X12 ) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} });
our @EXPORT = qw( );

our $VERSION = '0.01';

bootstrap EDI::Parser $VERSION;
use strict;
# Preloaded methods go here.

# **********************************************************************

sub new {
    my($pkg, %args) = @_;

    my $parser = $pkg->SUPER::new();
    
    $parser->setHandlers(%{$args{Handlers}}) if defined $args{Handlers};
    
    if(defined $args{Style} && $args{Style} eq 'Subs') {
	$parser->setHandlers(
			     Start   => \&handleStart,
			     End     => \&handleEnd,
			     Segment => \&handleSegment,
			     #Char    => \&handleChar,
			     );
    }
    
    $parser->{_Pkg} = defined $args{Pkg} ? $args{Pkg} : caller;

    return $parser;
}

sub handleStart {
    my($parser, $event, %parameters) = @_;
    no strict 'refs';
    
    my $method = $parser->{_Pkg} . "::$event";
    eval { &$method($parser, $event, %parameters) };
    return;
}

sub handleEnd {
    my($parser, $event, %parameters) = @_;
    no strict 'refs';

    my $method = $parser->{_Pkg} . "::${event}_";
    eval { &$method($parser, $event, %parameters) };
    return;
}

sub handleChar {
    my($parser, $chars) = @_;
    no strict 'refs';
    
    my $method = $parser->{_Pkg} . "::_char";
    eval { &$method($parser, $chars) };
    return;
}

sub handleSegment {
    my($parser, $segment, %parameters) = @_;
    no strict 'refs';
    
    my $method  = $parser->{_Pkg} . '::' . $segment->code;
    #my $method  = $parser->{_Pkg} . '::__' . $segment->code;
    my $default = $parser->{_Pkg} . '::SEGMENT';

    eval { &$default($parser, $segment, %parameters) }
    unless eval { &$method($parser, $segment, %parameters) };
    return;
}









package EDI::Parser::MEDICI;
sub new {
    my($pkg, %args) = @_;
    my($key, $val);
    my $parser = &parser_create();
    return undef unless $parser;
    my $obj = bless { _parser => $parser }, $pkg;
    &parser_user_data($parser, $obj);
    $obj->{'_'.$key} = $val while ($key, $val) = each(%args);
    return $obj;
}

sub DESTROY {
    my($self) = @_;
    &parser_free($self->{_parser}) if defined $self->{_parser};
}

sub reset {
    my($self) = @_;
    &parser_reset($self->{_parser}) if defined $self->{_parser};
}

sub error {
    my($self) = @_;
    return &parser_get_error_code($self->{_parser});
}

sub status {
    my($self) = @_;
    return &get_error_string($self->error);
}

sub abort {
    my($self) = @_;
    &parser_abort($self->{_parser});
}

sub type {
    my($self) = @_;
    return &parser_interchange_type($self->{_parser});
}

sub parse {
    my($self, $buffer, $done) = @_;
    $done = defined $done ? $done : 0;
    return &parser_parse($self->{_parser}, $buffer, $done ? 1 : 0);
}

sub xmltsg {
    my($self, $file) = @_;
    return undef unless defined $file;
    return &EDI::Parser::xmltsg_fd(fileno($file)) if ref($file) eq 'GLOB';
    return &EDI::Parser::xmltsg_file($file);
}

sub pyxtsg {
    my($self, $file) = @_;
    return undef unless defined $file;
    return &EDI::Parser::pyxtsg_fd(fileno($file)) if ref($file) eq 'GLOB';
    return &EDI::Parser::pyxtsg_file($file);
}

sub setHandlers {
    my($self, %handlers) = @_;
    while(my($key, $val) = each %handlers) {
	$self->{_Handlers}->{$key} = $val;
    }
}

sub startHandler {
    my($self, $event, $parameters) = @_;
    my $handler = $self->{_Handlers}->{Start};
    return &{$handler}($self, $event, %$parameters) if defined $handler;
}

sub endHandler {
    my($self, $event, $parameters) = @_;
    my $handler = $self->{_Handlers}->{End};
    #return &{$handler}($self, $event, %$parameters) if defined $handler;
    return &{$handler}($self, $event) if defined $handler;
}

sub charHandler {
    my($self, $chars) = @_;
    my $handler = $self->{_Handlers}->{Char};
    return &{$handler}($self, $chars) if defined $handler;
}

sub defaultHandler {
    my($self, $chars) = @_;
    my $handler = $self->{_Handlers}->{Default};
    return &{$handler}($self, $chars) if defined $handler;
}

sub segmentHandler {
    my($self, $segment, $directory, $parameters) = @_;
    my($handler);
    my %parameters = %$parameters;
    
    $segment = EDI::Parser::Segment->new($segment, $directory);
    $handler = $self->{_Handlers}->{Segment};
    return &{$handler}($self, $segment, %$parameters) if defined $handler;
}

sub tokenHandler {
    my($self, $token) = @_;
    my $handler = $self->{_Handlers}->{Token};
    
    $token = EDI::Parser::Token->new($token);
    return &{$handler}($self, $token) if defined $handler;
}

sub errorHandler {
    my($self, $code) = @_;
    my $string = &get_error_string($code);
    my $handler = $self->{_Handlers}->{Error};
    return &{$handler}($self, $code, $string) if defined $handler;
}

sub warningHandler {
    my($self, $code) = @_;
    my $string = &get_error_string($code);
    my $handler = $self->{_Handlers}->{Warning};
    return &{$handler}($self, $code, $string) if defined $handler;
}

sub directoryHandler {
    my($self, $parameters) = @_;
    my $return;
    my $handler = $self->{_Handlers}->{Directory};
    $return = &{$handler}($self, %$parameters) if defined $handler;
    return defined $return ? $return : 0;
}




package EDI::Parser::Segment;
sub new { bless { _segment => $_[1], _directory => $_[2] }, $_[0] }

sub code { &segment_get_code ($_[0]->{_segment}) }
#sub name { &directory_get_segment_name ($_[0]->{_directory}, $_[0]->code) }
sub name {
    &EDI::Parser::Directory::_itemInfo($_[0]->{_directory}, $_[0]->code, 0x10);
}

sub elementCount    { &segment_element_count   ($_[0]->{_segment}) }
sub subelementCount { &segment_subelement_count($_[0]->{_segment}, $_[1]) }

sub element {
    my($self, @indexes) = @_;
    my($index, @elements, $element, $subelement, $ref, $val);

    foreach $index (@indexes) {
	$val = undef;
	$ref = ref($index);
	
	if($ref eq 'ARRAY') {
	    $val = &segment_get_element	($self->{_segment}, @{$index});
	} else {
	    ($element, $subelement) = split(/\//, $index);
	    $val = &segment_get_element_by_name
		($self->{_segment}, $self->{_directory},
		 "$element\0", defined $subelement ? "$subelement\0" : "\0");
	}

	push(@elements, $val);
    }
    wantarray ? return @elements : $elements[0];
}

sub directory { return EDI::Parser::Directory->new($_[0]->{_directory}) }

package EDI::Parser::Directory;
sub new {
    my($pkg, $source) = @_;
    my $dir;
    
    if(ref($source) eq 'GLOB') {
	$dir = EDI::Parser::xmltsg_fd(fileno($source));
    } elsif($source =~ /^\d+$/) {
	$dir = $source;
    }

    return undef unless defined $dir && $dir;

    bless { _directory => $dir }, $pkg;
}

sub dir { $_[0]->{_directory} }

#sub elementName { _elementXXXX($_[0]->dir, $_[1], 0) }
#sub elementDesc { _elementXXXX($_[0]->dir, $_[1], 1) }
#sub elementNote { _elementXXXX($_[0]->dir, $_[1], 2) }
#sub segmentName { _segmentXXXX($_[0]->dir, $_[1], 0) }
#sub segmentDesc { _segmentXXXX($_[0]->dir, $_[1], 1) }
#sub segmentNote { _segmentXXXX($_[0]->dir, $_[1], 2) }
#sub compositeName { _compositeXXXX($_[0]->dir, $_[1], 0) }
#sub compositeDesc { _compositeXXXX($_[0]->dir, $_[1], 1) }
#sub compositeNote { _compositeXXXX($_[0]->dir, $_[1], 2) }


sub elementName   { _itemInfo ($_[0]->dir, $_[1], 0x00) }
sub elementDesc   { _itemInfo ($_[0]->dir, $_[1], 0x01) }
sub elementNote   { _itemInfo ($_[0]->dir, $_[1], 0x02) }
sub segmentName   { _itemInfo ($_[0]->dir, $_[1], 0x10) }
sub segmentDesc   { _itemInfo ($_[0]->dir, $_[1], 0x11) }
sub segmentNote   { _itemInfo ($_[0]->dir, $_[1], 0x12) }
sub compositeName { _itemInfo ($_[0]->dir, $_[1], 0x20) }
sub compositeDesc { _itemInfo ($_[0]->dir, $_[1], 0x21) }
sub compositeNote { _itemInfo ($_[0]->dir, $_[1], 0x22) }




sub elementType { _elementType($_[0]->dir, $_[1], 0) }
sub elementMin  { _elementType($_[0]->dir, $_[1], 1) }
sub elementMax  { _elementType($_[0]->dir, $_[1], 2) }

#sub segmentSize   { _segmentSize($_[0]->dir, $_[1]) }
#sub compositeSize { _compositeSize($_[0]->dir, $_[1]) }

sub segmentSize   { _itemSize ($_[0]->dir, $_[1], 0x10) }
sub compositeSize { _itemSize ($_[0]->dir, $_[1], 0x20) }

#sub segmentItem {
#    my($self, $code, $index) = @_;
#    my $a = _segmentItem($_[0]->dir, $code, $index);
#    return defined $a ? @$a : undef;
#};
#sub compositeItem {
#    my($self, $code, $index) = @_;
#    my $a = _compositeItem($_[0]->dir, $code, $index);
#    return defined $a ? @$a : undef;
#};


#sub segmentItem   {defined(my $r=_item($_[0]->dir,$_[1],$_[2]) ? @$r : undef) }
#sub compositeItem {defined(my $r=_item($_[0]->dir,$_[1],$_[2]) ? @$r : undef) }

sub segmentItem {
    my($self, $code, $index) = @_;
    my $a = _segmentItem($_[0]->dir, $code, $index);
    return defined $a ? @$a : undef;
};

sub compositeItem {
    my($self, $code, $index) = @_;
    my $a = _compositeItem($_[0]->dir, $code, $index);
    return defined $a ? @$a : undef;
};






























######################################################################
# Token was a bad idea and is obsolete ...
######################################################################


package EDI::Parser::Token;

sub new   { bless { _token => $_[1] }, $_[0] }
sub raw   { &token_get_rdata ($_[0]->{_token}) }
sub data  { &token_get_cdata ($_[0]->{_token}) }
sub type  { &token_get_type  ($_[0]->{_token}) }
sub last  { &token_is_last   ($_[0]->{_token}) }
sub first { &token_is_first  ($_[0]->{_token}) }

sub isElement { &token_is_el ($_[0]->{_token}) }
sub isTag     { &token_is_tg ($_[0]->{_token}) }
sub isST      { &token_is_st ($_[0]->{_token}) }
sub isTS      { &token_is_ts ($_[0]->{_token}) }
sub isSS      { &token_is_ss ($_[0]->{_token}) }
sub isES      { &token_is_es ($_[0]->{_token}) }

sub isText      { $_[0]->isElement || $_[0]->isTag }
sub isSeparator { $_[0]->isST || $_[0]->isTS || $_[0]->isSS || $_[0]->isES }




# **********************************************************************

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

EDI::Parser - Perl extension for the MEDICI EDI library

=head1 SYNOPSIS

 use EDI;

 $p1 = new EDI::Parser(Handlers => {Start   => \&handle_start,
				    End     => \&handle_end,
				    Segment => \&handle_segment,
				    Warning => \&handle_warning,
				    Error   => \&handle_error});

 $p1->parse($buffer, $done);

 $status = $p1->status;



=head1 DESCRIPTION

This module provides a way to parse EDI transmissions. It is built on
top of the MEDICI Electronic Data Interchange library.

=head1 THE API INTERFACE


=over 5




=item B<$value = $segment-E<gt>element($elemspec [, $elemspec]) ;>

Retrieves the value of the element specified by
C<$elemspec>. C<$elemspec> can be a scalar of the form
"<element-name>" or "<composite-name>/<element-name>" (eg. "1082" or
"C273/7008") or alternately an arrary reference specifying the indexes
of the required element, such as [0,0] for the first sub-element of
the first composite element, or [5,3] for the fourth sub-element of
the sixth composite element. The arrary reference form can be used
when no transaction-set guidelines are available to map element names
to segment position.

=back



=head1 EXPORTS

None by default. EDIFACT, UNGTDI and X12 may be imported.

=head1 AUTHOR

David Coles E<lt>david@disintegration.orgE<gt>

=head1 SEE ALSO

L<perl(1)>.

=cut
