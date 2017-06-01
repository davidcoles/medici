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

/*

  This program will display an EDI message in the following format:
  
  > <interchange>
  >     [UNB]=[UNOA][1]+[7349734757][12]+[5033075000007][14]+[990...
  >     <functionalgroup>
  >         [UNG]=[ORDERS]+[7349734757][12]+[5033075000007][14]+[99062...
  >         <transaction>
  >             [UNH]=[00000000035773]+[ORDERS][D][93A][UN]
  >             [BGM]=[220]
  >             [DTM]=[4][990621][101]
                ...
  >             [CNT]=[2][45]
  >             [UNT]=[35]+[00000000035799]
  >         [UNE]=[27]+[00000000000627]
  >     [UNZ]=[1]+[00000000000627]

  Where indentation denotes structure.
  
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <medici.h>


void starthandler (void *, EDI_Event, EDI_Parameters);
void endhandler (void *, EDI_Event);
void sephandler (void *, EDI_Event, char s);
void charhandler (void *, const char *, int);
void defaulthandler (void *, const char *, int);
void warninghandler (void *, int);

int
main (int argc, char **argv)
{
  EDI_Parser parser;
  FILE *stream = stdin;
  char buffer[8192];
  int done, userdata = 0;
  size_t length;

  /* If we were given a filename argument use that instead of stdin */

  if (argc > 1)
    if (!(stream = fopen (argv[1], "r")))
      {
	perror ("Couldn't open file");
	return 1;
      }

  /* First, create the parser  */

  if (!(parser = EDI_ParserCreate ()))
    {
      /* If this failed it's almost certainly because a malloc() failed */

      perror ("Couldn't create parser");
      return 1;
    }


  /* Set the userdata argument which will be passed to handlers */
  /* each time they are called. We will use this to indent output */

  EDI_SetUserData (parser, &userdata);

  /* Tell the parser to relax the rules on characters which are not */
  /* supposed to be allowed in the message - sometimes service advice */
  /* strings contain verboten characters such as newline */

  /* FIXME - make these activated by command line switches */
  /* EDI_SetPragma (parser, EDI_PCHARSET); */

  /* We want to be alerted if a error is handled pragmatically */
  /* so that we can display a warning message on stderr */

  EDI_SetWarningHandler (parser, warninghandler);

  /* Set up handlers which will indent and print output */

  /* These calls mark the start/end of "structural" data in the stream
   * - such as loops, segments, tags and elements, but not the
   * delimiting (separator) data.
   */     

  EDI_SetStartHandler (parser, starthandler);
  EDI_SetEndHandler (parser, endhandler);

  /* Character handler deals with all the data that would be important */
  /* to an application - segment tags and the contents of elements */
  /* (which is which can be worked out from the enclosing start/end */
  /* calls) */
  
  EDI_SetCharacterHandler (parser, charhandler);
  
  /* Delimiter characters can be read via EDI_SetSeparatorHandler */
  
  EDI_SetSeparatorHandler (parser, sephandler);

  /* Any characters not explicitly handled by other handlers gets */
  /* sent to the default handler. Only thing left after tags, */
  /* elements and separator characters so far is service string */
  /* advice*/

  EDI_SetDefaultHandler (parser, defaulthandler);


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

  fclose(stream);

  return 0;
}


void indent(int i) {
  int n;

  for (n = 0; n < i; n++)
    printf ("    ");
}

/* At the start of a enclosing envelope increase indentation */

void
starthandler (void *userdata, EDI_Event event, EDI_Parameters parameters)
{
  int *indentation = (int *) userdata;
  const char *code;

  switch(event)
    {
    case EDI_INTERCHANGE:
    case EDI_TRANSACTION:
    case EDI_GROUP:
    case EDI_LOOP:
      break;
    case EDI_SEGMENT:
      indent(*indentation);
      return;
    case EDI_ELEMENT:
    case EDI_TAG:
      printf("[");
      return;
    default:
      return;
    }
  
  indent((*indentation)++);

  if(parameters && (code = EDI_GetParameter (parameters, Code)))
    printf ("<%s %s>\n", EDI_GetEventString(event), code);
  else
    printf ("<%s>\n", EDI_GetEventString(event));
}


/* At the end of a enclosing envelope decrease indentation */

void
endhandler (void *userdata, EDI_Event event)
{
  int *indentation = (int *) userdata;

  switch(event)
    {
    case EDI_INTERCHANGE:
    case EDI_TRANSACTION:
    case EDI_GROUP:
    case EDI_LOOP:
      (*indentation)--;
      break;
    case EDI_SEGMENT:
      printf ("\n");
      break;
    case EDI_ELEMENT:
    case EDI_TAG:
      printf("]");
      break;
    default:
      break;
    }
}


void
sephandler (void *userdata, EDI_Event event, char s)
{
  switch(event)
    {
    case EDI_TS:
      printf("=");
      break;
    case EDI_ES:
      printf("+");
      break;
    default:
      break;
    }
  return;
}

void charhandler (void *u, const char *text, int size)
{
  printf("%.*s", size, text);
}

void defaulthandler (void *u, const char *text, int size)
{
  printf("{%.*s}\n", size, text);
}


/* If we encounter a non-fatal error alert the user */

void
warninghandler (void *u, int error)
{
  fprintf (stderr, "WARNING: %s\n", EDI_GetErrorString (error));
}
