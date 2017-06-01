use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

print <<EOF;

*** If you get an error like:
***
*** 	Could not eval '
***		    package ExtUtils::MakeMaker::_version;
***		    no strict;
*** 	<...>
***
***
*** Then you probably have an old version of perl and you will need to
*** modify Parser.pm - please see the note at the top of Parser.pm!!!

EOF

WriteMakefile(
	      'NAME'            => 'EDI::Parser',
	      'VERSION_FROM'    => 'Parser.pm', # finds $VERSION
	      'PREREQ_PM'       => {}, # e.g., Module::Name => 1.1
	      ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
	       (ABSTRACT_FROM   => 'Parser.pm', # retrieve abstract from module
		#'DEFINE'        => '-DHAS_CALL_METHOD -DHAS_SVPV_NOLEN',
		'DEFINE'        => '-DHAS_CALL_METHOD',
		AUTHOR          => 'David Coles <david@disintegration.org>')
	       : ()),
	      'LIBS'            => ['-L../src -lmedici -lexpat'],
	      # Insert -I. if you add *.h files later:
	      'INC'		=> '-I../src',
	      # Un-comment this if you add C files to link with later:
	      'OBJECT'           => '$(O_FILES)' .
				    ' ../examples/xmltsg.o' .
				    ' ../examples/expyx.o',
	      # link all the C files too
);
