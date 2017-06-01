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

/**********************************************************************
 * See xmltsg.c for how to embed a TSG into the executable
 **********************************************************************/

/* define some ASCII horizontal rules used for drawing boxes */

#define UL "_______________________________________________________________"
#define HR "---------------------------------------------------------------"

int evaluate = 1;


EDI_Directory directoryhandler(void *, EDI_Parameters);
void warninghandler (void *, int);
void errorhandler (void *, int);
void segmenthandler (void *, EDI_Parameters, EDI_Segment, EDI_Directory);
void starthandler (void *, EDI_Event, EDI_Parameters);
void endhandler (void *, EDI_Event);
int usage(void);


typedef struct {
  EDI_Directory directory;
  EDI_Parser parser;
  unsigned int indent;
}
MyData;

int
main (int argc, char **argv)
{
  EDI_Parser parser;
  FILE *stream = stdin;
  char buffer[4096], *xmlfile = NULL, *pyxfile = NULL;
  int optind;
  unsigned int length, done, n;
  MyData mydata;

  /* mydata will be registered as user data with the parser */
  /* we will use it to keep track of various variables */
  /* (which would otherwise have to be declared as globals) */

  mydata.directory = NULL;
  mydata.parser = NULL;
  mydata.indent = 0;


  /* Avoid using getopt(3) here for portability to non-unix systems. */
  /* This is only a demo, and there's no need for complex/long options */
  
  for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++)
    for (n = 1; n < strlen (argv[optind]); n++)
      switch (argv[optind][n])
	{
	case 'n':
	  evaluate = 0;
	  break;

	case 'x':
	  if(argv[optind][n+1] == '\0')
	    if(++optind < argc)
	      xmlfile = argv[optind];
	    else
	      exit ((usage (), -1));
	  else
	    xmlfile = argv[optind] + 2;
	  n = strlen (argv[optind]);
	  break;

	case 'p':
	  if(argv[optind][n+1] == '\0')
	    if(++optind < argc)
	      pyxfile = argv[optind];
	    else
	      exit ((usage (), -1));
	  else
	    pyxfile = argv[optind] + 2;
	  n = strlen (argv[optind]);
	  break;
	  
	case 'h':
	  exit ((usage (), 0));
	  
	default:
	  exit ((usage (), -1));
	}
  
  
  /* If we were given a tsg read it in */
  
  if(xmlfile)
    mydata.directory = read_xmltsg_file (xmlfile);
  else if(pyxfile)
    mydata.directory = read_pyxtsg_file (pyxfile);
  else if(xmlbuff)
    mydata.directory = read_xmltsg_buffer(xmlbuff, 0);
  else if(pyxbuff)
    mydata.directory = read_pyxtsg_buffer(pyxbuff, 0);
  
  /* If we were given a filename argument use that instead of stdin */

  if (optind < argc && !(stream = fopen (argv[optind], "r")))
    exit (((perror ("Couldn't open file")), -1));
  
  /* Create the parser */

  if (!(parser = EDI_ParserCreate ()))
    {
      /* If this failed it's almost certainly because a malloc() failed */
      
      perror ("Couldn't create parser");
      return 1;
    }
  
  /* Set the userdata argument which will be passed to handlers */
  /* each time they are called. */

  mydata.parser = parser;
  EDI_SetUserData (parser, &mydata);

  /* Tell the parser to relax the rules on characters which are not */
  /* supposed to be allowed in the message - sometimes service advice */
  /* strings contain verboten characters such as newline */
  
  /* FIXME - make these activated by command line switches */
  /* EDI_SetPragma (parser, EDI_PCHARSET | EDI_PSEGMENT); */

  /* We want to be alerted if a error is handled pragmatically */
  /* so that we can display a warning message on stderr */
  
  EDI_SetWarningHandler (parser, warninghandler);
  EDI_SetErrorHandler (parser, errorhandler);
  
  /* Set up handlers which will alter the indentation of output and */
  /* print a summary when structural changes occur in the document */
  
  EDI_SetStartHandler (parser, starthandler);
  EDI_SetEndHandler (parser, endhandler);
  
  /* When we encounter a full segment we want to be able to print out */
  /* the contents, so we set a handler for system and message segments */
  
  EDI_SetSegmentHandler (parser, segmenthandler);
  
  /* Set up a handler which will locate directory implementations */

  EDI_SetDirectoryHandler (parser, directoryhandler);

  /* Now we can start feeding the stream to the parser */

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
	return 1;
    }
  while (!done);
  
  /* Tell MEDICI to free all resources associated with this parse(r) */
  
  EDI_ParserFree (parser);

  if(mydata.directory)
    EDI_DirectoryFree(mydata.directory);

  fclose(stream);

  return 0;
}


void errorhandler (void *v, int i) {
  MyData *mydata = (MyData *) v;

  fprintf (stderr, "*** Error: Segment %ld: Character %ld: %s\n",
	   EDI_GetCurrentSegmentIndex (mydata->parser) + 1,
	   EDI_GetCurrentByteIndex (mydata->parser) + 1,
	   EDI_GetErrorString (i));
}


void warninghandler (void *v, int i) {
  MyData *mydata = (MyData *) v;

  fprintf (stderr,
	   "*** Warning: Segment %ld: Character %ld: %s\n",
	   EDI_GetCurrentSegmentIndex (mydata->parser) + 1,
	   EDI_GetCurrentByteIndex (mydata->parser) + 1,
	   EDI_GetErrorString (i));
}


void indentoutput (int m)
{
  int n;
  for (n = 0; n < m * 4; n++)
    printf(" ");
}


char *
itemtype (edi_item_t item)
{
  switch (item.repr)
    {
    case EDI_ISO2382A: return "a";
    case EDI_ISO2382N: return "n";
    case EDI_ISO2382X: return "an";
    case EDI_DECIMAL1: return "d1";
    case EDI_DECIMAL2: return "d2";
    case EDI_DECIMAL3: return "d3";
    case EDI_DECIMAL4: return "d4";
    case EDI_INTEGER: return "i";
    case EDI_REAL: return "r";
    default:
      return "??";
    }
}


EDI_Directory directoryhandler
(void *v, EDI_Parameters p)
{
  MyData *mydata = (MyData *) v;
  
  return mydata->directory;
}


void
starthandler (void *v, EDI_Event event, EDI_Parameters p)
{
  MyData *mydata = (MyData *) v;
  int n;
  char *s;

  switch(event)
    {
    case EDI_INTERCHANGE:
    case EDI_TRANSACTION:
    case EDI_GROUP:
    case EDI_LOOP:
      break;
    default:
      return;
    }

  indentoutput(mydata->indent++);
  printf ("[ start-of-%s", EDI_GetEventString(event));
  
  for (n = MinParameter; n < MaxParameter; n++)
    if ((s = EDI_GetParameter (p, n)) && strlen (s))
      printf (" %s=\"%s\"", EDI_GetParameterString (n), s);
  
  printf (" ]\n");
}


void endhandler
(void *v, EDI_Event event)
{
  MyData *mydata = (MyData *) v;

  switch(event)
    {
    case EDI_INTERCHANGE:
    case EDI_TRANSACTION:
    case EDI_GROUP:
    case EDI_LOOP:
      break;
    default:
      return;
    }
  
  --(mydata->indent);
  
  if (event == EDI_SECTION)
    return;
  
  indentoutput(mydata->indent);
  
  printf ("[ end-of-%s ]\n", EDI_GetEventString(event));
}


void elementhandler
(int indent, char *valu, int element, int subelement,
 EDI_Directory directory, char *code,  int is_subelement, int is_req)
{
  char tmp[64], eval = ' ';
  double a_double;
  unsigned int precision = 0, numeric = 0, n;
  edi_item_t item = EDI_NULL_ITEM;
  char *repr, *name, *desc, *clst;
  
  if(!valu)
    return;
  
  name = code ? EDI_ElementName (directory, code) : NULL;
  desc = code ? EDI_ElementDesc (directory, code) : NULL;
  clst = code ? EDI_GetCodelistValue (directory, code, valu) : NULL;
  
  if(code)
    item = EDI_ElementRepresentation (directory, code);
  
  switch (item.repr)
    {
    case EDI_ISO2382A:
      repr = "a";
      break;
    case EDI_ISO2382N:
      numeric = 1;
      repr = "n";
      break;
    case EDI_ISO2382X:
      repr = "an";
      break;
    case EDI_DECIMAL1:
      precision = 1;
      numeric = 1;
      repr = "d1";
      break;
    case EDI_DECIMAL2:
      precision = 2;
      numeric = 1;
      repr = "d2";
      break;
    case EDI_DECIMAL3:
      precision = 3;
      numeric = 1;
      repr = "d3";
      break;
    case EDI_DECIMAL4:
      precision = 4;
      numeric = 1;
      repr = "d4";
      break;
    case EDI_INTEGER:
      numeric = 1;
      repr = "i";
      break;
    case EDI_REAL:
      precision = 2;
      numeric = 1;
      repr = "r";
      break;
    default:
      repr = "??";
      break;
    }
  
  if(evaluate && numeric)
    {
      if(precision)
	{
	  EDI_EvaluateString(item.repr, valu, EDI_DOUBLE, &a_double);
	  sprintf(tmp, "%.*f",  precision, a_double);
	} else {
	  /* why, oh why have two different possible characters for 'point'? */
	  sprintf(tmp, "%s", valu);
	  for(n = 1; n <= strlen(tmp) && tmp[n-1] != '.'; n++)
	    if(tmp[n] == ',')
	      tmp[n] = '.';
	}
	  valu = tmp;
      eval = '#';
    }
  
  if(evaluate && clst)
    {
      sprintf(tmp, "%.10s - %.30s", valu, clst);
      valu = tmp;
      eval = '*';
    }
  
  if(!code)
    {
      sprintf(tmp, "#%02d:%02d", element, subelement);
      code = tmp;
    }
  
  indentoutput(indent);
  printf ((is_subelement ?
	   "|  %6.6s [%c%-35.35s ]( %2s%s%-2d %c )" :
	   "| %-6.6s  [%c%-35.35s ]( %2s%s%-2d %c )"),
	  code ? code : "????",
	  eval,
	  valu ? valu : "",
	  itemtype (item),
	  (item.max && item.min == item.max) ? "  " : "..",
	  item.max,
	  is_req ? 'M' : 'C');
  
  if (name)
    printf (" %s", name);
  
  if (desc && strlen (desc))
    printf (" - %s", desc);
  
  printf ("\n");
}


void segmenthandler
(void *v, EDI_Parameters parameters, EDI_Segment segment, EDI_Directory directory)
{
  MyData *mydata = (MyData *) v;
  int n, m;
  char *value, *code, *element, *name, *desc, *ctxt;
  edi_item_t item;

  /*if(EDI_GetParameter (parameters, ServiceSegment))
    directory = EDI_GetServiceDirectory(mydata->parser);*/

  /*directory = mydata->directory;*/

  code = EDI_GetCode (segment);
  name = EDI_SegmentName (directory, code);
  desc = EDI_SegmentDesc (directory, code);
  ctxt = EDI_GetParameter (parameters, Context);

  /* each segment is enclosed in a box for clarity - draw the upper line */

  indentoutput(mydata->indent);
  printf (" %s\n", UL);
  
  /* display the segment code and sequence number */

  indentoutput(mydata->indent);
  printf ("| %-3s [%ld]",
	  code ? code : "????",
	  EDI_GetCurrentSegmentIndex (mydata->parser) + 1);

  /* display the name of the segment if it was found in the directory */

  if (name && strlen (name))
    printf (" %s", name);
  
  /* display the description of the segment if it was found in the directory */
  
  if (desc && strlen (desc))
    printf (" - %s", desc);
  
  printf ("\n");
  
  /* display a context line if one was passed by the parser */
  
  if (ctxt)
    {
      indentoutput(mydata->indent);
      printf ("| %s\n", ctxt);
    }

  /* rule off the header section of the box */
  indentoutput(mydata->indent);
  printf ("|%s\n", HR);
  
    
  for (n = 0; n < EDI_GetElementCount(segment); n++)
    {
      item = EDI_SegmentItem(directory, code, n);
      element = item.code;
      
      if ((EDI_GetSubelementCount (segment, n) && item.type) ||
	  (EDI_GetSubelementCount (segment, n) > 1))
	{
	  name = element ? EDI_CompositeName (directory, element) : NULL;
	  desc = element ? EDI_CompositeDesc (directory, element) : NULL;

	  /* Output a line with composite name */
	  if(element)
	    {
	      indentoutput(mydata->indent);
	      printf ("| %-6.6s    %-35.35s  ( ------ %c ) %s",
		      element ,
		      "",
		      item.reqr ? 'M' : 'C',
		      name ? name : "");
	      
	      if(desc && strlen(desc))
		printf (" - %s", desc);
	      
	      printf("\n");
	    }
	  
	  for (m = 0; m < EDI_GetSubelementCount(segment, n); m++)
	    {
	      item = EDI_CompositeItem(directory, element, m);
	      
	      if((value = EDI_GetElement (segment, n, m)))
		elementhandler (mydata->indent, value, n, m,
				directory, item.code, 1, item.reqr);
	    }
	}
      else
	{
	  if((value = EDI_GetElement (segment, n, 0)))
	    elementhandler (mydata->indent, value, n, 0, 
			    directory, item.code, 0, item.reqr);
	}
    }
  
  /* finally, enclose the segment with the lower rule of the box */

  indentoutput(mydata->indent);
  printf ("|%s\n\n", UL);
}


int
usage (void)
{
  printf
    ("\n"
     "Usage: describe [-h] [-n] [-{x|p} <file>] [<edifile>]\n"
     "       -h this text\n"
     "       -n don't evaluate numeric or coded elements\n"
     "       -x read directory definition from <xmlfile>\n"
     "       -p read directory definition from <pyxfile>\n"
     "\n"
     "Reading an EDI stream from <edifile>, or stdin if no file is specified\n"
     "describe produces a \"human-readable\" summary of an EDI interchange.\n"
     "\n"
     "Structural events are indicated with text in square brackets, and\n");

  printf
    ("indentation shows how deeply nested each segment/event is.\n"
     "\n"
     "For a meaningful message description an appropriate directory\n"
     "definition needs to be provided with the -x or -t options.\n"
     "\n"
     "Segments are described as fully as possible with context sensitive\n"
     "information from the directories. Fields marked with an asterisk (*)\n"
     "indicate a coded value which has been looked up and a hash (#) symbol\n"
     "indicates a value which has been evaluated numerically.\n"
     "\n");

  return -1;
}


