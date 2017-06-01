/*
    
  The MEDICI Electronic Data Interchange Library
  Copyright (C) 2002  David Coles
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <medici.h>

#include "xmltsg.h"
#include "xmlout.h"

/**********************************************************************
 * When running with the -d -i flags (all detail, no indentation)
 * the XML markup may be stripped to leave what should be the original
 * message.
 * 
 * The following Perl command line can be used for stripping markup.
 * 
 * perl -mXML::Parser -e '(new XML::Parser( Handlers => 
 * { Char => sub {print $_[1]} }))->parse(\*STDIN)'
 * 
 * Also see xmltsg.c for how to embed a TSG into the executable
 **********************************************************************/

typedef struct {
  int indent;
} userdata_t;

int command_line_options(int, char **);
int skip (EDI_Event);
void indent(int);
void errorhandler (void *, int);
void warninghandler (void *, int);
void defaulthandler (void *u, const char *, int);
void charhandler (void *u, const char *, int);
void separatorhandler (void *u, EDI_Event,  char);
void starthandler (void *, EDI_Event, EDI_Parameters);
void endhandler (void *, EDI_Event);
EDI_Directory directoryhandler(void *, EDI_Parameters);




/* Globals for command-line processing, etc. */

EDI_Directory directory = NULL;
char *xmlfile = NULL;
char *pyxfile = NULL;
int terse = 0;
int pretty = 1;
int detail = 1;




int
main (int argc, char **argv)
{
  EDI_Parser parser;
  FILE *stream = stdin;
  char buffer[4096];
  int arg, done;
  size_t length;
  userdata_t userdata;

  memset(&userdata, 0, sizeof(userdata));
  
  arg = command_line_options(argc, argv);
  
  /* If we were given a filename argument use that instead of stdin */
  
  if (arg && arg < argc)
    if (!(stream = fopen (argv[arg], "r")))
      {
	sprintf(buffer, "Couldn't open file '%s'", argv[arg]);
	perror (buffer);
	exit(-1);
      }
  
  /* Parse the TSG if embedded or given on the command line */
  
  if(xmlfile)
    directory = read_xmltsg_file (xmlfile);
  else if(pyxfile)
    directory = read_pyxtsg_file (pyxfile);
  else if(xmlbuff) 
    directory = read_xmltsg_buffer(xmlbuff, 0);
  else if(pyxbuff) 
    directory = read_pyxtsg_buffer(pyxbuff, 0);

  
  /* Create the parser  */
  
  if (!(parser = EDI_ParserCreate ()))
    {
      /* If this failed it's almost certainly because a malloc() failed */
      
      perror ("Couldn't create parser");
      exit(-1);
    }
  
  /* Set the userdata argument which will be passed to handlers */
  /* each time they are called. We will use this to indent output */
  
  EDI_SetUserData (parser, &userdata);
  
  /* Tell the parser to relax the rules on characters which are not */
  /* supposed to be allowed in the message - sometimes service advice */
  /* strings contain verboten characters such as newline */

  /* FIXME - make these activated by command line switches */
  /* EDI_SetPragma (parser, EDI_PCHARSET); */
  EDI_SetPragma (parser, EDI_PTUNKNOWN | EDI_PCORRUPT | EDI_PREQUIRED);

  /* We want to be alerted if a error is handled (maybe pragmatically) */
  /* so that we can display a warning message on stderr */
  
  EDI_SetErrorHandler (parser, errorhandler);
  EDI_SetWarningHandler (parser, warninghandler);
  
  /* Set up handlers which will indent output */

  EDI_SetStartHandler (parser, starthandler);
  EDI_SetEndHandler (parser, endhandler);

  /* We want to pass out the text of the message as the XML CDATA */

  EDI_SetCharacterHandler (parser, charhandler);

  if(detail > 1)
    {
      EDI_SetDefaultHandler (parser, defaulthandler);
      EDI_SetSeparatorHandler (parser, separatorhandler);
    }
  

  /* Tell the parser how to find out about transaction set directories */
  /* This allows it to understand the structure of the transaction */
  
  EDI_SetDirectoryHandler (parser, directoryhandler);
  
  /* Print the xml header */
  vxmlprocessinginstruction ("xml",
			     "version", "1.0",
			     "encoding", "ISO-8859-1",
			     "standalone", "yes",
			     NULL);
  xmlprint ("\n");
  
  /* Now we can start feeding the stream to MEDICI */

  do
    {
      /* Read in the stream chunk by chunk. If we don't get a whole */
      /* buffer full of characters the stream is finished */
      
      length = fread (buffer, 1, sizeof (buffer), stream);
      done = length < sizeof (buffer);

      /* Tell MEDICI to parse this chunk. If it is the last chunk */
      /* the "done" argument will be 1 to indicate this to MEDICI */

      EDI_Parse (parser, buffer, length, done);
      
      /* The error code will be non-zero on a fatal error */

      if(EDI_GetErrorCode (parser))
	{
	  fprintf (stderr, "%s at segment %ld\n",
		   EDI_GetErrorString (EDI_GetErrorCode (parser)),
		   EDI_GetCurrentSegmentIndex (parser));
	  return 1;
	}
    }
  while (!done);

  /* Tell MEDICI to free all resources associated with this parse(r) */

  EDI_ParserFree (parser);

  if(directory)
    EDI_DirectoryFree(directory);

  fclose(stream);

  return 0;
}


/* Only one directory to use - it was read in at the start */

EDI_Directory directoryhandler
(void *v, EDI_Parameters p)
{
  return directory;
}

/* If we encounter a warning alert the user */

void
errorhandler (void *u, int error)
{
  fprintf (stderr, "\nERROR: %s\n", EDI_GetErrorString (error));
}

void
warninghandler (void *u, int error)
{
  fprintf (stderr, "\nWARNING: %s\n", EDI_GetErrorString (error));
}

void
charhandler (void *u, const char *text, int size)
{
  if(detail)
    xmlprintn(text, size);
}

void
defaulthandler (void *u, const char *text, int size)
{
  xmlprintn(text, size);
}

void
separatorhandler (void *u, EDI_Event event,  char separator)
{
  userdata_t *userdata = (userdata_t *) u;
  
  if(pretty && EDI_RI != event)
    indent(userdata->indent);

  xmlstartelement(EDI_GetEventString(event), NULL);
  xmlprintn(&separator, 1);
  xmlendelement(EDI_GetEventString(event));

  if(pretty && EDI_RI != event)
    xmlprint("\n");

  return;
}








int skip (EDI_Event event)
{
  switch(event)
    {
    case EDI_INTERCHANGE:
    case EDI_GROUP:
    case EDI_TRANSACTION:
    case EDI_LOOP:
    case EDI_SEGMENT:
      return 0;
      
    case EDI_COMPOSITE:
    case EDI_ELEMENT:
    case EDI_TAG:
      return detail < 1;

    case EDI_TS:
    case EDI_ES:
    case EDI_SS:
    case EDI_ST:
    case EDI_RI:
    case EDI_ADVICE:
      return detail < 2;
    default:
      break;
    }
  
  return detail < 1;
}

int split (EDI_Event event)
{
  switch(event)
    {
      /* never split */
    case EDI_ADVICE:
    case EDI_ELEMENT:
    case EDI_TAG:
      return 0;
      
      /* 'segment' should not be split if we are not being verbose */
    case EDI_SEGMENT:
      return (detail >= 1) ? 1 : 0;
      
    default:
      break;
    }
  
  return 1;
}



/* At the start of a section print an XML element and increase indentation */

void
starthandler (void *u, EDI_Event event, EDI_Parameters parameters)
{
  userdata_t *userdata = (userdata_t *) u;
  void (*function)(char *, ...) = vxmlstartelement;

  if(skip(event))
    return;
  
  if(pretty && EDI_RI != event)
    indent(userdata->indent);
  
  if(!detail && EDI_SEGMENT == event)
    function = vxmlvoidelement;
  
#define GP(p) EDI_GetParameter (parameters, p)
  
  function (EDI_GetEventString(event),
	    
	    /* interchange */
	    "standard", GP(Standard),			/* eg. EDIFACT */
	    "sender", GP(SendersId),			/* eg. 9999999999991 */
	    "recipient", GP(RecipientsId),		/* eg. 9999999999992 */
	    "date", GP(Date),				/* eg. 021121 */
	    "time", GP(Time),				/* eg. 0909 */
	    "test", GP(TestIndicator),			/* eg. 1 */
	    
	    /* transaction */
	    "reference", GP(MessageReferenceNumber),	/* eg. 1 */
	    "type", GP(MessageType),			/* eg. INVOIC */
	    "version", GP(MessageVersionNumber),	/* eg. D */
	    "release", GP(MessageReleaseNumber),	/* eg. 97A */
	    "agency", GP(ControllingAgency),		/* eg. UN */
	    
	    /* segment/composite/element */
	    "code", GP(Code),
	    "name", terse ? NULL : GP(Name),
	    "desc", terse ? NULL : GP(Desc),
	    "note", terse ? NULL : GP(Note),
	    "list", terse ? NULL : GP(List),
	    
	    /* don't think these get used */
	    "transactioncode", GP(TransactionCode),
	    "transactiontype", GP(TransactionType),

	    /* appear in service advice */
	    "ts", terse ? NULL : GP(TagSeparator),
	    "dn", terse ? NULL : GP(DecimalNotation),
	    "ri", terse ? NULL : GP(ReleaseIndicator),
	    "rs", terse ? NULL : GP(RepetitionSeparator),
	    "st", terse ? NULL : GP(SegmentTerminator),
	    "es", terse ? NULL : GP(ElementSeparator),
	    "ss", terse ? NULL : GP(SubelementSeparator),
	    
	    "element", terse ? NULL : GP(Element),
	    "subelement", terse ? NULL : GP(Subelement),
	    
	    NULL);
#undef GP
  
  if(split(event) && pretty)
    xmlprint ("\n");
  
  userdata->indent++;
}


/* At the end of a section end the XML element and decrease indentation */

void
endhandler (void *u, EDI_Event event)
{
  userdata_t *userdata = (userdata_t *) u;

  if(skip(event))
    return;
  
  userdata->indent--;

  if(split(event) && pretty)
    indent(userdata->indent);

  if(detail || EDI_SEGMENT != event)
    xmlendelement (EDI_GetEventString(event));

  if(EDI_RI != event && pretty)
    xmlprint ("\n");
}






void indent(int i)
{
  int n;
  for (n = 0; n < i; n++)
    xmlprint ("  ");
}


int
usage (int n)
{
  printf
    ("\n"
     "Usage: editoxml [-t] [-o] [-i] [-v] [-{x|p} <file>] [<edifile>]\n"
     "       editoxml -h\n"
     "\n"
     "       -h help (this text)\n"
     "       -t terse output - less attributes\n"
     "       -o outline only - no data elements, just structure\n"
     "       -i no indentation or newlines\n"
     "       -d detailed output - separator characters as well as elements\n"
     "       -x read directory definition from <xmlfile>\n"
     "       -p read directory definition from <pyxfile>\n"
     "\n");
  return n;
}

/* very simple command line options - can't be bothered with getopt(3) */

int command_line_options(int argc, char **argv) 
{
  int optind;
  
  for (optind = 1; argv[optind] && argv[optind][0] == '-'; optind++)
    {
      
      /* switches must be a minus followed by exactly one letter */
      
      if(argv[optind][1] == '\0' || argv[optind][2] != '\0')
	exit (usage (-1));
      
      switch (argv[optind][1])
	{
	  
	case 'x':
	  if(!(xmlfile = argv[++optind]))
	    exit (usage (-1));
	  break;

	case 'p':
	  if(!(pyxfile = argv[++optind]))
	    exit (usage (-1));
	  break;
	  
	case 't':
	  terse = 1;
	  break;

	case 'o':
	  detail = 0;
	  break;

	case 'i':
	  pretty = 0;
	  break;

	case 'd':
	  detail = 2;
	  break;
	  
	case 'h':
	  exit (usage (0));
	  
	default:
	  exit (usage (-1));
	}
    }
  
  return optind;
}









